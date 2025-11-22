#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstdlib>
#include <vector>
#include <string>
#include <sstream>
#include <unistd.h>      // geteuid()
#include <sys/types.h>   // uid_t

namespace fs = std::filesystem;

// =======================================================
// Helper functions
// =======================================================

bool is_absolute_path(const std::string &path) {
    return !path.empty() && path[0] == '/';
}

void require_root() {
    if (geteuid() != 0) {
        std::cerr << "Error: you must run this command as root (sudo/doas)." << std::endl;
        exit(1);
    }
}

void require_absolute(const std::string &path) {
    if (!is_absolute_path(path)) {
        std::cerr << "Error: package path must be an *absolute* path." << std::endl;
        std::cerr << "Example: sudo pms install /home/user/test.pms" << std::endl;
        exit(1);
    }
}

// =======================================================
// Package metadata
// =======================================================

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

// =======================================================
// Install package
// =======================================================

int install_package(const std::string &pkg_path) {
    require_root();            // ← MUST BE ROOT
    require_absolute(pkg_path); // ← PATH MUST BE ABSOLUTE

    fs::path tmp = "/tmp/pms_extract";
    fs::remove_all(tmp);
    fs::create_directories(tmp);

    // Extract using 'ar'
    std::string ar_cmd =
        "cd " + tmp.string() + " && ar -x " + pkg_path;

    if (system(ar_cmd.c_str()) != 0) {
        std::cerr << "Error: ar failed to extract package (file may not exist)" << std::endl;
        return 1;
    }

    // Expect data.tar
    fs::path data_tar = tmp / "data.tar";
    if (!fs::exists(data_tar)) {
        std::cerr << "Malformed package: data.tar missing" << std::endl;
        return 1;
    }

    // Extract data.tar
    std::string tar_cmd = "tar -xf " + data_tar.string() + " -C /";
    if (system(tar_cmd.c_str()) != 0) {
        std::cerr << "Failed to extract data.tar" << std::endl;
        return 1;
    }

    // Read metadata
    PackageInfo info = read_control(tmp / "control");
    std::cout << "Installed: " << info.name << " " << info.version << std::endl;

    // Save to database
    fs::create_directories("/var/lib/pms/db");
    std::ofstream db("/var/lib/pms/db/" + info.name);
    db << info.version << std::endl;
    for (auto &dep : info.dependencies) db << dep << std::endl;

    return 0;
}

// =======================================================
// Main entry
// =======================================================

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
