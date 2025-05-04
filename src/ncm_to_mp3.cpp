// src/ncm_to_mp3.cpp

#include "ncm_to_mp3.h"

#include <fstream>
#include <vector>
#include <cstring>
#include <cstdint>
#include <cctype>
#include <string>
#include <openssl/aes.h>

static std::vector<unsigned char> base64_decode(const std::vector<unsigned char>& input) {
    static const std::string b64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    auto is_b64 = [](unsigned char c)->bool {
        return std::isalnum(c) || (c == '+') || (c == '/');
    };

    std::vector<unsigned char> out;
    unsigned char char4[4], char3[3];
    int len = input.size();
    int i = 0, j = 0;
    int in_ = 0;

    while (len-- && input[in_] != '=' && is_b64(input[in_])) {
        char4[i++] = input[in_]; in_++;
        if (i == 4) {
            for (i = 0; i < 4; ++i)
                char4[i] = b64_chars.find(char4[i]);
            char3[0] = (char4[0] << 2) + ((char4[1] & 0x30) >> 4);
            char3[1] = ((char4[1] & 0xf) << 4) + ((char4[2] & 0x3c) >> 2);
            char3[2] = ((char4[2] & 0x3) << 6) +  char4[3];
            for (i = 0; i < 3; ++i)
                out.push_back(char3[i]);
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; ++j) char4[j] = 0;
        for (j = 0; j < 4; ++j) char4[j] = b64_chars.find(char4[j]);
        char3[0] = (char4[0] << 2) + ((char4[1] & 0x30) >> 4);
        char3[1] = ((char4[1] & 0xf) << 4) + ((char4[2] & 0x3c) >> 2);
        char3[2] = ((char4[2] & 0x3) << 6) +  char4[3];
        for (j = 0; j < i - 1; ++j)
            out.push_back(char3[j]);
    }

    return out;
}

bool convert_ncm_to_mp3(const std::string& inPath, const std::string& outPath) {
    std::ifstream ifs(inPath, std::ios::binary);
    if (!ifs) return false;

    // 1. 读取并验证 magic
    char magic[8];
    ifs.read(magic, 8);
    if (std::memcmp(magic, "CTENFDAR", 8) != 0) return false;

    // 2. 读取 meta 长度并 xor 0x64
    uint32_t metaLen = 0;
    ifs.read(reinterpret_cast<char*>(&metaLen), 4);
    std::vector<unsigned char> metaBuf(metaLen);
    ifs.read(reinterpret_cast<char*>(metaBuf.data()), metaLen);
    for (auto& b : metaBuf) b ^= 0x64;

    // 3. Base64 解码
    auto metaDec = base64_decode(metaBuf);

    // 4. AES-128-ECB 解密
    static const unsigned char aeskey[16] = {
        0x23,0x31,0x34,0x6C,0x6A,0x6B,0x5F,0x21,
        0x5C,0x5D,0x26,0x30,0x55,0x3C,0x27,0x28
    };
    AES_KEY key;
    AES_set_decrypt_key(aeskey, 128, &key);

    std::vector<unsigned char> plain(metaDec.size());
    for (size_t i = 0; i < metaDec.size(); i += 16) {
        AES_ecb_encrypt(metaDec.data() + i,
                        plain.data()   + i,
                        &key, AES_DECRYPT);
    }

    // 5. 解析 coreKey 并 xor 0x63
    uint32_t coreKeyLen = *reinterpret_cast<uint32_t*>(plain.data() + 0x10);
    std::vector<unsigned char> coreKey(
        plain.begin() + 0x10 + 4,
        plain.begin() + 0x10 + 4 + coreKeyLen
    );
    for (auto& b : coreKey) b ^= 0x63;

    // 6. 初始化 RC4-like keyBox
    std::vector<unsigned char> box(256), rndkey(256);
    for (int i = 0; i < 256; ++i) {
        box[i] = i;
        rndkey[i] = coreKey[i % coreKey.size()];
    }
    int j = 0;
    for (int i = 0; i < 256; ++i) {
        j = (j + box[i] + rndkey[i]) & 0xFF;
        std::swap(box[i], box[j]);
    }

    // 7. 解密剩余音频数据
    std::ofstream ofs(outPath, std::ios::binary);
    unsigned char buf[4096];
    size_t idx = 0;
    while (ifs) {
        ifs.read(reinterpret_cast<char*>(buf), sizeof(buf));
        std::streamsize r = ifs.gcount();
        for (std::streamsize k = 0; k < r; ++k) {
            idx = (idx + 1) & 0xFF;
            j   = (j + box[idx]) & 0xFF;
            std::swap(box[idx], box[j]);
            unsigned char t = (box[idx] + box[j]) & 0xFF;
            buf[k] ^= box[t];
        }
        ofs.write(reinterpret_cast<char*>(buf), r);
    }
    return true;
}
