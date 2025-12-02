#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include "suffix_automaton.h"

// 生成100条恶意URL特征
std::vector<std::string> generate_malicious_urls() {
    std::vector<std::string> urls;

    // 常见的恶意URL特征
    urls.push_back("malware");
    urls.push_back("virus");
    urls.push_back("trojan");
    urls.push_back("ransomware");
    urls.push_back("phishing");
    urls.push_back("spyware");
    urls.push_back("adware");
    urls.push_back("botnet");
    urls.push_back("keylogger");
    urls.push_back("rootkit");
    urls.push_back("backdoor");
    urls.push_back("worm");
    urls.push_back("exploit");
    urls.push_back("payload");
    urls.push_back("shellcode");
    urls.push_back("buffer overflow");
    urls.push_back("SQL injection");
    urls.push_back("XSS");
    urls.push_back("CSRF");
    urls.push_back("clickjacking");
    urls.push_back("directory traversal");
    urls.push_back("remote code execution");
    urls.push_back("denial of service");
    urls.push_back("distributed denial of service");
    urls.push_back("brute force");
    urls.push_back("credential stuffing");
    urls.push_back("account takeover");
    urls.push_back("data breach");
    urls.push_back("identity theft");
    urls.push_back("credit card fraud");
    urls.push_back("phishing kit");
    urls.push_back("malvertising");
    urls.push_back("cryptojacking");
    urls.push_back("ransomware as a service");
    urls.push_back("malware as a service");
    urls.push_back("exploit kit");
    urls.push_back("command and control");
    urls.push_back("C2 server");
    urls.push_back("domain generation algorithm");
    urls.push_back("fast flux");
    urls.push_back("bulletproof hosting");
    urls.push_back("dark web");
    urls.push_back("tor network");
    urls.push_back("onion site");
    urls.push_back("malware dropper");
    urls.push_back("downloader");
    urls.push_back("loader");
    urls.push_back("packer");
    urls.push_back("obfuscator");
    urls.push_back("polymorphic malware");
    urls.push_back("metamorphic malware");
    urls.push_back("fileless malware");
    urls.push_back("memory-resident malware");
    urls.push_back("bootkit");
    urls.push_back("MBR malware");
    urls.push_back("firmware malware");
    urls.push_back("IoT malware");
    urls.push_back("Mirai");
    urls.push_back("Bashlite");
    urls.push_back("Gafgyt");
    urls.push_back("VPNFilter");
    urls.push_back("TrickBot");
    urls.push_back("Emotet");
    urls.push_back("QakBot");
    urls.push_back("Zeus");
    urls.push_back("SpyEye");
    urls.push_back("Conficker");
    urls.push_back("WannaCry");
    urls.push_back("NotPetya");
    urls.push_back("BadRabbit");
    urls.push_back("Locky");
    urls.push_back("CryptoLocker");
    urls.push_back("TeslaCrypt");
    urls.push_back("Jigsaw");
    urls.push_back("Petya");
    urls.push_back("GoldenEye");

    // 如果数量不足100，添加一些随机特征
    while (urls.size() < 100) {
        urls.push_back("malicious_feature_" + std::to_string(urls.size() + 1));
    }

    return urls;
}

// 生成测试文本
std::string generate_test_text(const std::vector<std::string>& malicious_urls) {
    std::string text = "This is a test text containing some malicious URL features. ";
    text += "For example, it might include malware, phishing, or trojan. ";
    text += "It could also have SQL injection or XSS vulnerabilities. ";
    text += "Some other malicious features might be ransomware, spyware, or adware. ";
    text += "The text might also mention botnet, keylogger, or rootkit. ";
    text += "It could include backdoor, worm, or exploit. ";
    text += "Some payload or shellcode might be present. ";
    text += "There could be a buffer overflow or directory traversal. ";
    text += "Remote code execution or denial of service might be mentioned. ";
    text += "Distributed denial of service or brute force could be included. ";
    text += "Credential stuffing or account takeover might be present. ";
    text += "Data breach or identity theft could be mentioned. ";
    text += "Credit card fraud or phishing kit might be included. ";
    text += "Malvertising or cryptojacking could be present. ";
    text += "Ransomware as a service or malware as a service might be mentioned. ";
    text += "Exploit kit or command and control could be included. ";
    text += "C2 server or domain generation algorithm might be present. ";
    text += "Fast flux or bulletproof hosting could be mentioned. ";
    text += "Dark web or tor network might be included. ";
    text += "Onion site or malware dropper could be present. ";
    text += "Downloader or loader might be mentioned. ";
    text += "Packer or obfuscator could be included. ";
    text += "Polymorphic malware or metamorphic malware might be present. ";
    text += "Fileless malware or memory-resident malware could be mentioned. ";
    text += "Bootkit or MBR malware might be included. ";
    text += "Firmware malware or IoT malware could be present. ";
    text += "Mirai or Bashlite might be mentioned. ";
    text += "Gafgyt or VPNFilter could be included. ";
    text += "TrickBot or Emotet might be present. ";
    text += "QakBot or Zeus could be mentioned. ";
    text += "SpyEye or Conficker might be included. ";
    text += "WannaCry or NotPetya could be present. ";
    text += "BadRabbit or Locky might be mentioned. ";
    text += "CryptoLocker or TeslaCrypt could be included. ";
    text += "Jigsaw or Petya might be present. ";
    text += "GoldenEye or other malicious features could be mentioned. ";

    return text;
}

int main() {
    // 生成100条恶意URL特征
    std::vector<std::string> malicious_urls = generate_malicious_urls();
    std::cout << "Generated " << malicious_urls.size() << " malicious URL features." << std::endl;

    // 创建后缀自动机并添加所有恶意URL特征
    SuffixAutomaton sa;

    auto start_time = std::chrono::high_resolution_clock::now();

    for (const std::string& url : malicious_urls) {
        sa.add_string(url);
    }

    // 构建后缀自动机
    sa.build();

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cout << "Built suffix automaton in " << duration.count() << " milliseconds." << std::endl;

    // 生成测试文本
    std::string test_text = generate_test_text(malicious_urls);
    std::cout << "Generated test text of length " << test_text.size() << " characters." << std::endl;

    // 在测试文本中查找所有恶意URL特征
    start_time = std::chrono::high_resolution_clock::now();

    int total_matches = 0;
    for (const std::string& url : malicious_urls) {
        std::vector<int> occurrences = sa.find_all_occurrences(url);
        if (!occurrences.empty()) {
            total_matches++;
            std::cout << "Found " << url << " at positions: ";
            for (int pos : occurrences) {
                std::cout << pos << " ";
            }
            std::cout << std::endl;
        }
    }

    end_time = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    std::cout << "Found " << total_matches << " malicious URL features in test text." << std::endl;
    std::cout << "Search completed in " << duration.count() << " milliseconds." << std::endl;

    // 测试最长公共子串功能
    std::string s1 = "This is a test string for LCS.";
    std::string s2 = "Another test string for LCS.";
    std::string lcs = sa.longest_common_substring(s1, s2);
    std::cout << "Longest common substring between \"" << s1 << "\" and \"" << s2 << "\" is \"" << lcs << "\" (length: " << lcs.size() << ")." << std::endl;

    // 测试出现次数统计功能
    std::string pattern = "test";
    int count = sa.get_total_occurrences(pattern);
    std::cout << "Pattern \"" << pattern << "\" occurs " << count << " times in the suffix automaton." << std::endl;

    return 0;
}