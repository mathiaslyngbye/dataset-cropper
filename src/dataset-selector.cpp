// OpenCV includes
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>

// Standard includes
#include <random>
#include <vector>
#include <iostream>
#include <string>
#include <regex>
#include <experimental/filesystem>

#define VERBOSE false

bool compare(const std::pair<float,cv::Mat>&i, const std::pair<float,cv::Mat>&j)
{
    return i.first > j.first;
}

float lightness(cv::Mat img)
{
    cv::Mat image = img;
    if( image.empty() )
    {
        std::cout <<  "Could not open or find the image" << std::endl ;
        return -1;
    }

    cv::cvtColor( image, image, cv::COLOR_BGR2GRAY );
    cv::equalizeHist( image, image );

    int pixels = 0;
    int64_t sum = 0;
    for(size_t row = 0; row < image.rows; row++)
    {
        for(size_t col = 0; col < image.cols; col++)
        {
            uchar intensity = image.at<uchar>(row,col);
            sum += intensity;
        }
    }

    float avg = (sum*1.0)/(image.cols*image.rows);

    if(VERBOSE)
        std::cout << "Lightness:\t\t" << avg << std::endl;

    return avg;
}

float occupancy(cv::Mat img, int threshold)
{
    cv::Mat image = img;
    if( image.empty() )
    {
        std::cout <<  "Could not open or find the image" << std::endl ;
        return -1;
    }

    cv::cvtColor( image, image, cv::COLOR_BGR2GRAY );
    //cv::equalizeHist( image, image );

    int pixels = 0;
    for(size_t row = 0; row < image.rows; row++)
    {
        for(size_t col = 0; col < image.cols; col++)
        {
            uchar intensity = image.at<uchar>(row,col);
            if(intensity > threshold)
                pixels++;
        }
    }

    float pct = (pixels*100.0)/(image.cols*image.rows);

    if(VERBOSE)
        std::cout << "occupancy (%):\t" << pct << std::endl;

    return pct;
}

float similarity(cv::Mat img1, cv::Mat img2)
{
    cv::Mat image1 = img1;
    cv::Mat image2 = img2;
    if( image1.empty() || image2.empty())
    {
        std::cout <<  "Could not open or find the image" << std::endl ;
        return -1;
    }

    cv::cvtColor( image1, image1, cv::COLOR_BGR2GRAY );
    cv::cvtColor( image2, image2, cv::COLOR_BGR2GRAY );
    cv::equalizeHist( image1, image1 );
    cv::equalizeHist( image2, image2 );

    int sum = 0;
    for(size_t row = 0; row < image1.rows; row++)
    {
        for(size_t col = 0; col < image1.cols; col++)
        {
            uchar intensity1 = image1.at<uchar>(row,col);
            uchar intensity2 = image2.at<uchar>(row,col);

            sum += std::abs(intensity2-intensity1);
        }
    }
    float pct = 100-((sum*100.0)/(255*image1.rows*image1.cols));

    if(VERBOSE)
        std::cout << "Similarity (%):\t" << pct << std::endl;

    return pct;
}


void selection_first(std::vector<cv::Mat> input_images, std::vector<cv::Mat> &output_images, int count)
{
    output_images.clear();
    for(int i = 0; i < count; i++)
        output_images.push_back(input_images[i]);
}

void selection_last(std::vector<cv::Mat> input_images, std::vector<cv::Mat> &output_images, int count)
{
    output_images.clear();
    for(int i = input_images.size()-count; i < input_images.size(); i++)
        output_images.push_back(input_images[i]);
}

void selection_middle(std::vector<cv::Mat> input_images, std::vector<cv::Mat> &output_images, int count)
{
    output_images.clear();
    int start_index = (input_images.size()/2)-(count/2);
    if(VERBOSE)
        std::cout << "Start index: " << start_index << std::endl;

    for(int i = start_index; i < start_index+count; i++)
        output_images.push_back(input_images[i]);
}

void selection_random(std::vector<cv::Mat> input_images, std::vector<cv::Mat> &output_images, int count)
{
    std::random_device device;
    std::default_random_engine generator(device());

    output_images.clear();
    for(int i = 0; i < count; i++)
    {
        int distribution_size = input_images.size()-1;
        std::uniform_int_distribution<int> distribution(0,distribution_size);

        int index = distribution(generator);

        // Debug out
        if(VERBOSE)
        {
            std::cout << "Choice: " << index << std::endl;
            std::cout << distribution_size << "->" << distribution_size-1 << std::endl;
        }

        // Save and remove element
        output_images.push_back(input_images[index]);
        input_images.erase(input_images.begin() + index);
    }
}

void selection_best(std::vector<cv::Mat> input_images, std::vector<cv::Mat> &output_images, int count)
{
    std::vector<std::pair<float, cv::Mat>> scores;

    for(int i = 0; i < input_images.size(); i++)
    {
        float occupancyScore = occupancy(input_images[i],0);
        float lightnessScore = lightness(input_images[i]);

        float similarityScore = 0;
        if(i == 0)
            similarityScore = similarity(input_images[i],input_images[i+1]);
        else
            similarityScore = similarity(input_images[i-1],input_images[i]);

        float metric = 5*occupancyScore+5*lightnessScore+10*similarityScore;

        scores.push_back(std::make_pair(metric,input_images[i]));
    }

    std::sort(scores.begin(), scores.end(), compare);

    for(int i = 0; i < count; i++)
    {
        output_images.push_back(scores[i].second);
    }
}

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

int main(int argc, char *argv[])
{
    std::array<std::string,5> types = {"first", "last", "middle", "random", "best" };
    // Receive input
    if(argc != 4)
    {
        std::cout << "Usage:" << std::endl;
        std::cout << "Command: ./dataset-selector <dataset path> <selection type> <selection count>" << std::endl;
        std::cout << "Example (best 5 images): ./dataset-selector <dataset path> 4 5" << std::endl << std::endl;
        std::cout << "Selection types:" << std::endl;
        std::cout << "0: First image(s)" << std::endl;
        std::cout << "1: Last image(s)" << std::endl;
        std::cout << "2: Middle image(s)" << std::endl;
        std::cout << "3: Random image(s)" << std::endl;
        std::cout << "4: Best* image(s)" << std::endl;

        return 0;
    }

    std::string arg_path(argv[1]);
    int arg_type = std::atoi(argv[2]);
    int arg_count = atoi(argv[3]);
    std::cout << "Dataset: " << arg_path << std::endl;
    std::cout << "Selection type: " << arg_type << " (" << types[arg_type] << ")" << std::endl;
    std::cout << "Selection count: " << arg_count << std::endl;

    // Remove last slash
    char last_char = arg_path[arg_path.length()-1];
    if(last_char == '/')
         arg_path = arg_path.substr(0, arg_path.length() - 1);

    // Create output directories
    std::string path_selected = arg_path + "_selection_" + types[arg_type];
    std::experimental::filesystem::create_directory(path_selected);

    // Directory iterator
    int export_index = 0;
    for (const auto & entry : std::experimental::filesystem::directory_iterator(argv[1]))
    {
        //std::cout << entry << std::endl;
        std::string path_string = entry.path();

        // Create category subdirectories
        int slash_index_dir = path_string.find_last_of("\\/")+1;
        std::string category_dir_name = path_string.substr(slash_index_dir,path_string.length()-slash_index_dir);
        std::string category_dir_selected = path_selected+"/"+category_dir_name;
        std::experimental::filesystem::create_directory(category_dir_selected);

        // Fetch all images of directory (alphabetically)
        std::vector<std::string> image_paths;
        fetch_image_paths(path_string,image_paths);
        std::sort(image_paths.begin(), image_paths.end());

        // Make decision
        std::vector<cv::Mat> image_sequence;

        int max_value = image_paths.size();
        if(max_value%20)
        {
            std::cout << "Warning: Bad maximum value. Ignoring redundant images..." << std::endl;
            std::cout << "> Path: " << path_string << std::endl;
            max_value = image_paths.size()-(image_paths.size()%20);
        }

        for(int i = 0; i < max_value; i++)
        {
            //std::cout << "Index: " << i << std::endl;
            image_sequence.push_back(cv::imread(image_paths[i], cv::IMREAD_COLOR ));

            if(!((i+1)%20))
            {
                // Select desired images
                std::vector<cv::Mat> image_selection;

                switch(arg_type)
                {
                case 0:
                    selection_first(image_sequence, image_selection, arg_count);
                    break;
                case 1:
                    selection_last(image_sequence, image_selection, arg_count);
                    break;
                case 2:
                    selection_middle(image_sequence, image_selection, arg_count);
                    break;
                case 3:
                    selection_random(image_sequence, image_selection, arg_count);
                    break;
                case 4:
                    selection_best(image_sequence, image_selection, arg_count);
                    break;
                default:
                    std::cout << "Invalid selection type: " << arg_type << std::endl;
                    break;
                }

                // Debug output
                if(VERBOSE)
                {
                    std::cout << "Resetting..." << std::endl;
                    std::cout << "Sequence size:\t"<< image_sequence.size() << std::endl;
                    std::cout << "Selection size:\t"<< image_selection.size() << std::endl;
                }

                for(int i = 0; i < image_selection.size(); i++)
                {
                    char index_padded[25];
                    sprintf(index_padded, "%05d", export_index);
                    std::string image_name = category_dir_selected+"/image_"+index_padded+".png";
                    cv::imwrite(image_name, image_selection[i]);
                    export_index++;
                }

                image_sequence.clear();
            }
        }
    }

    std::cout << "End of main!" << std::endl;
    return 0;
}
