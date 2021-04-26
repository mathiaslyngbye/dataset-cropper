// OpenCV includes
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>

// Standard includes
#include <vector>
#include <iostream>
#include <string>
#include <regex>
#include <experimental/filesystem>
#include <random>

// Defines
#define VERBOSE false
#define EXPORT_RANDOM false
#define EXPORT_RANDOM_COUNT 4

void fetch_image_paths(std::string path, std::vector<std::string> &image_paths)
{
    if(VERBOSE)
        std::cout << "Dataset path \"" << path << "\"..." << std::endl;

    // Generate vector of paths to supported files
    image_paths.clear();
    std::vector<std::string> supported_file_extensions = {"jpg", "png"};
    for (const auto & entry : std::experimental::filesystem::directory_iterator(path))
    {
        std::string path_string = entry.path();
        std::string file_extension = path_string.substr(path_string.find_last_of(".") + 1);

        for(const auto & supported_file_extension : supported_file_extensions)
        {
            if(file_extension == supported_file_extension)
            {
                image_paths.push_back(path_string);
                break;
            }
        }
    }

    if(VERBOSE)
        std::cout << "Found " << image_paths.size() << " image(s)!" << std::endl;
}

cv::Mat threshold(const cv::Mat &image_src, int threshold = 80)
{
    cv::Mat image_grey, image_dst;
    image_dst = image_src.clone();
    cv::cvtColor(image_src, image_grey, CV_BGR2GRAY);

    for(int row = 0; row < image_src.rows; row++)
    {
        for(int col = 0; col < image_src.cols; col++)
        {
            int pixel_intensity = image_grey.at<uchar>(row,col);
            if(pixel_intensity < threshold)
            {
                cv::Vec3b &color = image_dst.at<cv::Vec3b>(row,col);
                color[0] = 0;
                color[1] = 0;
                color[2] = 0;
            }
        }
    }

    return image_dst;
}

bool is_filled(const cv::Mat &image_src)
{
    cv::Mat image_grey;
    cv::cvtColor(image_src, image_grey, CV_BGR2GRAY);

    for(int row = 0; row < image_src.rows; row++)
    {
        for(int col = 0; col < image_src.cols; col++)
        {
            int pixel_intensity = image_grey.at<uchar>(row,col);
            if(pixel_intensity == 0)
            {
                return false;
            }
        }
    }

    return true;
}

void chop_image(const cv::Mat &image, std::vector<cv::Mat> &sub_images, int width, int height)
{

    cv::Mat image_th = threshold(image, 80);

    for(int row = 0; row < image.rows/height; row++)
    {
        for(int col = 0; col < image.cols/width; col++)
        {
            cv::Rect image_roi(col*width, row*height, width, height);
            cv::Mat sub_image = image_th(image_roi);
            if(is_filled(sub_image))
                sub_images.push_back(sub_image);
        }
    }
}

int main(int argc, char *argv[])
{
    std::cout << "argc = " << argc << std::endl;
    for(int i = 0; i < argc; i++)
        std::cout << "argv[" << i << "] = " << argv[i] << std::endl;

    std::string arg_path(argv[1]);

    // Remove last slash
    char last_char = arg_path[arg_path.length()-1];
    if(last_char == '/')
         arg_path = arg_path.substr(0, arg_path.length() - 1);

    std::string path_chopped = arg_path + "_chopped";
    std::experimental::filesystem::create_directory(path_chopped);

    int export_index = 0;
    for (const auto & entry : std::experimental::filesystem::directory_iterator(arg_path))
    {
        std::string path_string = entry.path();

        int slash_index_dir = path_string.find_last_of("\\/")+1;
        std::string category_dir_name = path_string.substr(slash_index_dir,path_string.length()-slash_index_dir);
        std::string category_dir_chopped = path_chopped+"/"+category_dir_name;
        std::experimental::filesystem::create_directory(category_dir_chopped);

        if(VERBOSE)
            std::cout << "Scanning category: " << category_dir_name << std::endl;

        std::vector<std::string> image_paths;
        fetch_image_paths(path_string,image_paths);
        std::sort(image_paths.begin(), image_paths.end());


        for(int i = 0; i < image_paths.size(); i++)
        {
            //int slash_index_file = image_paths[i].find_last_of("\\/")+1;
            //std::cout << image_paths[i].substr(slash_index_file,image_paths[i].length()-slash_index_dir) << std::endl;
            cv::Mat image = cv::imread(image_paths[i], cv::IMREAD_COLOR );
            if(VERBOSE)
                std::cout << image_paths[i] << std::endl;

            std::vector<cv::Mat> sub_images;
            chop_image(image, sub_images, 224, 224);

            if(EXPORT_RANDOM)
            {
                std::random_device device;
                std::default_random_engine generator(device());

                for(int i = 0; i < EXPORT_RANDOM_COUNT; i++)
                {
                    int distribution_size = sub_images.size()-1;
                    std::uniform_int_distribution<int> distribution(0,distribution_size);

                    int index = distribution(generator);

                    if(VERBOSE)
                    {
                        std::cout << "Selection: " << index << "/" << distribution_size << std::endl;
                    }

                    char index_padded[25];
                    sprintf(index_padded, "%05d", export_index);
                    std::string image_name = category_dir_chopped+"/image_"+index_padded+".png";
                    cv::imwrite(image_name, sub_images[index]);

                    sub_images.erase(sub_images.begin() + index);
                    export_index++;
                }

            }
            else // Export all
            {
                for(int i = 0; i < sub_images.size(); i++)
                {
                    //cv::imshow("Test", sub_images[i]);
                    //cv::waitKey(0);

                    char index_padded[25];
                    sprintf(index_padded, "%05d", export_index);

                    std::string image_name = category_dir_chopped+"/image_"+index_padded+".png";
                    cv::imwrite(image_name, sub_images[i]);
                    export_index++;
                }
            }
        }
    }

    std::cout << "End of main!" << std::endl;
    return 0;
}
