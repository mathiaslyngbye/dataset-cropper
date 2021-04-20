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

int main(int argc, char *argv[])
{
    std::cout << "End of main!" << std::endl;
    return 0;
}
