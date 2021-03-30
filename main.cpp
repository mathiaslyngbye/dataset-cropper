// OpenCV includes
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>

// Standard includes
#include <vector>
#include <iostream>
#include <string>
#include <regex>
#include <experimental/filesystem>

void fetch_image_paths(std::string path, std::vector<std::string> &image_paths)
{
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
    std::cout << "Found " << image_paths.size() << " image(s)!" << std::endl;
}

cv::Mat crop_image(cv::Mat image)
{
    cv::Mat image_grey;
    cv::cvtColor(image, image_grey, CV_BGR2GRAY);

    int min_row = image.rows, max_row = 0, min_col = image.cols, max_col = 0;
    for(int row = 0; row < image.rows; row++)
    {
        for(int col = 0; col < image.cols; col++)
        {
            int pixel = image_grey.at<uchar>(row,col);
            if(pixel != 0)
            {
                if(row < min_row)
                    min_row = row;
                if(row > max_row)
                    max_row = row;
                if(col < min_col)
                    min_col = col;
                if(col > max_col)
                    max_col = col;
            }
        }
    }

    int min_height = 224;
    int min_width = 224;

    if(max_col-min_col < min_width)
    {
        if((max_col-min_width) >= 0)
            min_col = max_col-min_width;
        else if (min_col+min_width < image.cols)
            max_col = min_col+min_width;
        else
            std::cout << "I should not be here" << std::endl;
    }

    if(max_row-min_row < min_height)
    {
        if (min_row+min_height < image.rows)
            max_row = min_row+min_width;
        else if((max_row-min_height) >= 0)
            min_row = max_row-min_height;
        else
            std::cout << "I should not be here" << std::endl;
    }

    int crop_width = max_col - min_col;
    int crop_height = max_row - min_row;


    //std::cout << "Min col: " << min_col << "\tMax col: " << max_col << "\tDiff: " << max_col-min_col << std::endl;
    //std::cout << "Min row: " << min_row << "\tMax row: " << max_row << "\tDiff: " << max_row-min_row << std::endl;

    cv::Rect image_roi(min_col, min_row, crop_width, crop_height);
    image = image(image_roi);

    return image;
}

int main(int argc, char *argv[])
{
    std::cout << "argc = " << argc << std::endl;
    for(int i = 0; i < argc; i++)
        std::cout << "argv[" << i << "] = " << argv[i] << std::endl;

    std::string path(argv[1]);
    std::string path_cropped = path + "_cropped";
    std::experimental::filesystem::create_directory(path_cropped);

    for (const auto & entry : std::experimental::filesystem::directory_iterator(argv[1]))
    {
        std::string path_string = entry.path();

        int slash_index_dir = path_string.find_last_of("\\/")+1;
        std::string category_dir_name = path_string.substr(slash_index_dir,path_string.length()-slash_index_dir);
        std::string category_dir_cropped = path_cropped+"/"+category_dir_name;
        std::experimental::filesystem::create_directory(category_dir_cropped);

        std::cout << "Scanning category: " << category_dir_name << std::endl;

        std::vector<std::string> image_paths;
        fetch_image_paths(path_string,image_paths);
        std::sort(image_paths.begin(), image_paths.end());


        for(int i = 0; i < image_paths.size(); i++)
        {
            //int slash_index_file = image_paths[i].find_last_of("\\/")+1;
            //std::cout << image_paths[i].substr(slash_index_file,image_paths[i].length()-slash_index_dir) << std::endl;
            cv::Mat image = cv::imread(image_paths[i], cv::IMREAD_COLOR );
            cv::Mat image_cropped = crop_image(image);

            char index_padded[25];
            sprintf(index_padded, "%05d", i);

            std::string image_name = category_dir_cropped+"/image_"+index_padded+".png";
            cv::imwrite(image_name, image_cropped);
        }
    }


    std::cout << "End of main!" << std::endl;
    return 0;
}
