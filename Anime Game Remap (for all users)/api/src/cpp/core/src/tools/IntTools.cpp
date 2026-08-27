#include "AGRemapCore/tools/IntTools.h"

#include <string>
#include <z3++.h>
#include <tsl/ordered_map.h>
#include <xxhash.h>
#include "compressonator.h"
#include <curl/curl.h>
#include <iostream>
#include "AGRemapCore/tools/grapheme/GraphemeRange.h"
#include <string>

static const unsigned int Base64BaseNum = 64;
static const std::vector<std::string> Base64Digits = {"A", "B", "C", "D", "E", "F", "G", "H", 
                                               "I", "J", "K", "L", "M", "N", "O", "P", 
                                               "Q", "R", "S", "T", "U", "V", "W", "X", 
                                               "Y", "Z", "a", "b", "c", "d", "e", "f", 
                                               "g", "h", "i", "j", "k", "l", "m", "n", 
                                               "o", "p", "q", "r", "s", "t", "u", "v", 
                                               "w", "x", "y", "z", "0", "1", "2", "3", 
                                               "4", "5", "6", "7", "8", "9", "+", "_"};


namespace AGRemapCore {
    std::vector<unsigned int> IntTools::toBase(long long num, unsigned int base, bool *isNegative, bool *error) {
        std::vector<unsigned int> digits;

        if (base <= 1) {
            *error = true;
            *isNegative = false;
            return digits;
        }

        *error = false;

        if (num == 0) {
            *isNegative = false;
            digits.push_back(0);
            return digits;
        }

        *isNegative = num < 0;
        if (*isNegative) {
            num *= -1;
        }

        unsigned int digit;

        while (num) {
            digit = num % base;
            digits.push_back(digit);
            num /= base;
        }

        std::reverse(digits.begin(), digits.end());
        return digits;
    }

    std::string IntTools::toStrBase(long long num, unsigned int base, const std::vector<std::string>& getDigit, const std::string& negativeChar, bool *error) {
        bool isNegative;
        std::string result;

        std::vector<unsigned int> digits = IntTools::toBase(num, base, &isNegative, error);
        if (*error) {
            return result;
        }

        size_t resultLen = isNegative ? negativeChar.length() : 0;

        for (auto digit: digits) {
            resultLen += getDigit[digit].length();
        }

        result.reserve(resultLen);

        if (isNegative) {
            result += negativeChar;
        }

        for (auto digit : digits) {
            result += getDigit[digit];
        }

        return result;
    }

    static void testFunc() {
        z3::context ctx;
        z3::solver solver(ctx);

        // Create variables
        z3::expr x = ctx.int_const("x");
        z3::expr y = ctx.int_const("y");

        // Add constraints:
        solver.add(x + y == 10);
        solver.add(x > 0);
        solver.add(y > 0);
        solver.add(x < 10);
        solver.add(y < 10);

        std::cout << "Checking constraints...\n";

        switch (solver.check()) {
            case z3::sat: {
                std::cout << "SAT\n";

                z3::model m = solver.get_model();
                std::cout << "x = " << m.eval(x) << "\n";
                std::cout << "y = " << m.eval(y) << "\n";
                break;
            }
            case z3::unsat:
                std::cout << "UNSAT\n";
                break;

            case z3::unknown:
                std::cout << "UNKNOWN\n";
                break;
        }
    }

    static void testFunc2() {
       std::string text = "a👨‍👩‍👧‍👦👍🏽é";
       std::cout << text << '\n';

        for (std::string_view grapheme : GraphemeRange(text)) {
            std::cout << "[" << grapheme << "]\n";
        }

        for (std::string_view grapheme : GraphemeRange(text)) {
            for (unsigned char c : grapheme)
                std::printf("%02X ", c);

            std::printf("\n");
        }
    }

    static void testFunc3() {
        tsl::ordered_map<std::string, int> map = {
            {"Charlie", 300},
            {"Alice", 150}
        };

        map["one"] = 1;
        map["two"] = 2;
        map["three"] = 3;

        for (const auto &[k, v] : map)
        {
            std::cout << k << ' ' << v << '\n';
        }
    }

    static void testFunc4() {
        std::string_view text = "hello world";

        XXH128_hash_t hash =
            XXH3_128bits(text.data(), text.size());

        std::cout
            << "high: " << hash.high64 << '\n'
            << "low:  " << hash.low64 << '\n';
    }

    static void testFunc6() {
        std::cout << "Compressonator sanity test\n";

        CMP_InitFramework();

        std::cout << "Compressonator initialized successfully!\n";
    }

    static void testFunc7() {
        std::cout << "libcurl sanity test\n";

        CURLcode result = curl_global_init(CURL_GLOBAL_DEFAULT);

        if (result != CURLE_OK) {
            std::cerr << "curl_global_init failed: "
                    << curl_easy_strerror(result)
                    << '\n';
            return;
        }

        CURL* curl = curl_easy_init();

        if (curl == nullptr) {
            std::cerr << "curl_easy_init failed\n";
            curl_global_cleanup();
            return;
        }

        std::string response;

        curl_easy_setopt(curl, CURLOPT_URL, "https://api.github.com/");

        curl_easy_setopt(
            curl,
            CURLOPT_WRITEFUNCTION,
            [](char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t {
                auto* output = static_cast<std::string*>(userdata);
                output->append(ptr, size * nmemb);
                return size * nmemb;
            }
        );

        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        curl_easy_setopt(
            curl,
            CURLOPT_USERAGENT,
            "AGRemapCore-sanity-test"
        );

        result = curl_easy_perform(curl);

        if (result != CURLE_OK) {
            std::cerr << "curl_easy_perform failed: "
                    << curl_easy_strerror(result)
                    << '\n';

            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return;
        }

        long responseCode = 0;

        curl_easy_getinfo(
            curl,
            CURLINFO_RESPONSE_CODE,
            &responseCode
        );

        std::cout << "HTTP response code: "
                << responseCode
                << '\n';

        std::cout << "Downloaded "
                << response.size()
                << " bytes\n";

        std::cout << "Response:\n"
                << response
                << '\n';

        curl_easy_cleanup(curl);
        curl_global_cleanup();
    }

    std::string IntTools::toBase64(long long num, bool *error, const std::optional<std::vector<std::string>>& getDigit, const std::string& negativeChar) {
        // testFunc();
        // testFunc2();
        // testFunc3();
        // testFunc4();
        // testFunc6();
        // testFunc7();
        return IntTools::toStrBase(num, Base64BaseNum, getDigit.has_value() ? *getDigit : Base64Digits, negativeChar, error);
    }
}