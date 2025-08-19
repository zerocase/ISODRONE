/****************************************************************
          libscala-file, (C) 2020 Mark Conway Wirt
            See LICENSE for licensing terms (MIT)
 ****************************************************************/

#include <iostream>
#include <fstream>
#include <regex>
#include <math.h>
#include <stdexcept>

#include "ScalaFile.h"
#include "JuceHeader.h"

namespace scala {
    scale read_scl(std::ifstream& input_file){ 
        /* 
        C++ Code to parse the Scala scale file, as documented here:
            http://www.huygens-fokker.org/scala/scl_format.html
        */
        
        int non_comments_processed = 0;  // Fixed typo: non_commnets -> non_comments
        unsigned long entries = 0;
        int numerator, denominator;
        std::string buffer;
        std::smatch match, trimmed;
        scale scala_scale;
        bool description_parsed = false;

#ifdef SCALA_STRICT
        std::regex COMMENT_REGEX = std::regex(R"(^\s*!.*)");
#else
        std::regex COMMENT_REGEX = std::regex(R"(^\s*!.*)");
#endif

        while (std::getline(input_file, buffer)) {
            DBG("Processing line: '" + juce::String(buffer) + "'");
            
            // Trim trailing whitespace and newlines
            buffer.erase(buffer.find_last_not_of(" \t\r\n") + 1);
            
            if (std::regex_match(buffer, COMMENT_REGEX)) {
                DBG("-> Comment line");
                continue;  // Skip comment lines entirely
            } 
            else if (buffer.empty() || std::regex_match(buffer, std::regex(R"(^\s*$)"))) {
                DBG("-> Blank line");
                // Handle blank lines
#ifdef SCALA_STRICT
                if (description_parsed && non_comments_processed > 1) {
                    // If we're past the header and hit a blank line, we're done
                    break;
                }
#endif
                continue;  // Skip blank lines
            } 
            else {
                DBG("-> Content line");
                // Extract the part after optional leading whitespace and before optional trailing content
                std::regex_search(buffer, trimmed, std::regex(R"(^\s*([^\s]+).*)"));
                std::string entry = trimmed.size() > 1 ? trimmed[1] : buffer;
                DBG("Extracted entry: '" + juce::String(entry) + "'");
                
                if (non_comments_processed == 0) {
                    DBG("-> Description line");
                    // First non-comment is the description
                    description_parsed = true;
                    non_comments_processed++;
                    continue;
                }
                else if (non_comments_processed == 1) {
                    // Second non-comment line contains the number of entries
                    DBG("Parsing entry count: '" + juce::String(entry) + "'");
                    try {
                        entries = std::stoul(entry);
                        DBG("Successfully parsed entries: " + juce::String((int)entries));
                    } catch (const std::exception& e) {
                        DBG("Failed to parse entry count '" + juce::String(entry) + "': " + juce::String(e.what()));
                        throw;
                    }
                    non_comments_processed++;
                    continue;
                }
                
                // Parse scale degrees
                if (std::regex_match(entry, std::regex(R"(.*/.+)"))) {
                    // A ratio (e.g., "3/2", "5/4")
                    std::regex_search(entry, match, std::regex(R"((\d+)/(\d+))"));
                    if (match.size() >= 3) {
                        numerator = std::stoi(match.str(1)); 
                        denominator = std::stoi(match.str(2)); 
                        scala_scale.add_degree(*new degree(numerator, denominator));
                        DBG("Added ratio: " + juce::String(numerator) + "/" + juce::String(denominator));
                    }
                }
                else if (std::regex_match(entry, std::regex(R"(\d*\.\d+)"))) {
                    // Cent values with decimal point (e.g., "100.0", "272.727")
                    double cents = std::stod(entry);
                    scala_scale.add_degree(*new degree(cents));
                    DBG("Added cents: " + juce::String(cents));
                }
                else if (std::regex_match(entry, std::regex(R"(^\d+$)"))) {
                    // Integer values treated as ratios (e.g., "2" becomes "2/1")
                    numerator = std::stoi(entry); 
                    denominator = 1;
                    scala_scale.add_degree(*new degree(numerator, denominator));
                    DBG("Added integer ratio: " + juce::String(numerator) + "/1");
                }
#ifdef SCALA_STRICT
                else {
                    // In strict mode, throw an error if the line can't be interpreted
                    std::string message = "Scala parse error: cannot interpret: ";
                    throw std::runtime_error(message.append(buffer));
                }
#else
                else {
                    DBG("Skipping unrecognized entry: '" + juce::String(entry) + "'");
                }
#endif
                non_comments_processed++;
            }
        }
        
#ifdef SCALA_STRICT
        if (scala_scale.get_scale_length() != entries + 1) {
            // Check that we parsed the expected number of entries
            throw std::runtime_error("Scala file parse error: Unexpected number of entries. Expected: " + 
                                    std::to_string(entries + 1) + ", Got: " + 
                                    std::to_string(scala_scale.get_scale_length()));
        }
#endif
        
        return scala_scale;
    }
} // namespace scala