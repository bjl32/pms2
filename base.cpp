#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <vector>
#include <string>
#include <sstream>

namespace fs = std::filesystem;

// Very primitive package metadata
struct PackageInfo {
    std::string name;
    std::string version;
    std::vector<std::string> dependencies;
};

// Reads metadata from control file in extracted dir
PackageInfo read_control(const fs::path &control_path) {
    PackageInfo info;
    std::ifstream f(control_path);
    std::string line;

    while (std::getline(f, line)) {
        if (line.rfind("Name:", 0) == 0) {
            info.name = line.substr(5);
        } else if (line.rfind("Version:", 0) == 0) {
            info.version = line.substr(8);
        } else if (line.rfind("Depends:", 0) == 0) {
            std::stringstream ss(line.substr(8));
            std::string dep;
            while (std::getline(ss, dep, ',')) {
                if (!dep.empty()) info.dependencies.push_back(dep);
            }
        }
    }
    return info;
}

// Simple install: extract and copy files
int install_package(const std::string &pkg_path) {
    fs::path tmp = "/tmp/pms_extract";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    // Extract using 'ar'
    std::string ar_cmd = "ar -xf " + pkg_path + " -C /tmp/pms_extract";
    if (system(ar_cmd.c_str()) != 0) {
        std::cerr << "Failed to extract package" << std::endl;
        return 1;
    }

    // Expect data.tar
    fs::path data_tar = tmp / "data.tar";
    if (!fs::exists(data_tar)) {
        std::cerr << "Malformed package: no data.tar" << std::endl;
        return 1;
    }

    // Extract data.tar
    std::string tar_cmd = "tar -xf " + data_tar.string() + " -C /";
    if (system(tar_cmd.c_str()) != 0) {
        std::cerr << "Failed to extract data.tar" << std::endl;
        return 1;
    }

    // Read control metadata
    PackageInfo info = read_control(tmp / "control");
    std::cout << "Installed: " << info.name << " " << info.version << std::endl;

    // Save to database
    fs::create_directories("/var/lib/pms/db");
    std::ofstream db("/var/lib/pms/db/" + info.name);
    db << info.version << std::endl;
    for (auto &dep : info.dependencies) db << dep << std::endl;

    return 0;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        std::cout << "Usage: pms install <package.pms>" << std::endl;
        return 0;
    }

    std::string cmd = argv[1];
    if (cmd == "install") {
        return install_package(argv[2]);
    }

    std::cerr << "Unknown command" << std::endl;
    return 1;
}
