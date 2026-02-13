#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <cstdlib>
#include <algorithm>
#include <limits>

namespace fs = std::filesystem;

void wait_for_key() {
    std::cout << "\n按任意键继续...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

int main() {
    std::cout << "RDR2截图提取解密工具" << std::endl;

    const char* home_env = std::getenv("USERPROFILE");
    if (!home_env) home_env = std::getenv("HOME");
    if (!home_env) {
        std::cerr << "ERROR:找不到用户文件夹" << std::endl;
        wait_for_key();
        return 1;
    }
    fs::path home = home_env;
    fs::path profiles = home / "Documents" / "Rockstar Games" / "Red Dead Redemption 2" / "Profiles";

    fs::path src_dir;
    try {
        for (auto const& d : fs::directory_iterator(profiles)) {
            if (!d.is_directory()) continue;
            bool has_prdr = false;
            for (auto const& f : fs::directory_iterator(d.path())) {
                if (f.path().filename().string().rfind("PRDR", 0) == 0) {
                    has_prdr = true;
                    break;
                }
            }
            if (has_prdr) {
                src_dir = d.path();
                break;
            }
        }
    } catch (const fs::filesystem_error&) {
        std::cerr << "ERROR:无法访问RDR2配置文件夹" << std::endl;
        wait_for_key();
        return 1;
    }

    if (src_dir.empty()) {
        std::cerr << "ERROR:找不到RDR2截图文件夹" << std::endl;
        wait_for_key();
        return 1;
    }

    std::string dst_str;
    std::cout << "请键入保存路径: ";
    std::getline(std::cin, dst_str);
    fs::path dst_dir = dst_str;
    if (!fs::exists(dst_dir)) fs::create_directories(dst_dir);

    int ok = 0, fail = 0;
    const char jpeg_header[] = {'\xff', '\xd8'};

    for (auto const& f : fs::directory_iterator(src_dir)) {
        auto fname = f.path().filename().string();
        if (fname.rfind("PRDR", 0) != 0) continue;

        std::ifstream fi(f.path(), std::ios::binary);
        if (!fi) { ++fail; continue; }
        std::vector<char> buf((std::istreambuf_iterator<char>(fi)), {});
        fi.close();

        auto it = std::search(buf.begin(), buf.end(),
                              std::begin(jpeg_header), std::end(jpeg_header));
        if (it == buf.end()) { ++fail; continue; }

        fs::path out_path = dst_dir / (fname + ".jpeg");
        std::ofstream fo(out_path, std::ios::binary);
        if (!fo) { ++fail; continue; }
        fo.write(&*it, buf.end() - it);
        fo.close();
        ++ok;
    }

    std::cout << "成功: " << ok << "  失败: " << fail << std::endl;
    wait_for_key();
    return 0;
}