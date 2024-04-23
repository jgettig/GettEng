#ifndef ENGINE_UTILS_H
#define ENGINE_UTILS_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "rapidjson/filereadstream.h"
#include "rapidjson/document.h"

using std::cout;
using std::endl;

class EngineUtils
{
public:

	static void ReadJsonFile(const std::string& path, rapidjson::Document& out_document) {
		FILE* file_pointer = nullptr;
#ifdef _WIN32
		fopen_s(&file_pointer, path.c_str(), "rb");
#else
		file_pointer = fopen(path.c_str(), "rb");
#endif

		char buffer[65536];
		rapidjson::FileReadStream stream(file_pointer, buffer, sizeof(buffer));
		out_document.ParseStream(stream);
		std::fclose(file_pointer);

		if (out_document.HasParseError()) {
			rapidjson::ParseErrorCode errorcode = out_document.GetParseError();
			cout << "error parsing json at [" << path << "]" << endl;
			exit(0);
		}
	}

	static void PrintMessageFromDocument(const std::string& member, rapidjson::Document& d) {
		std::string out = "";
		if (d.HasMember(member.c_str()) && d[member.c_str()].IsString())
			out = d[member.c_str()].GetString();
		cout << out;
	}

	static std::string obtain_word_after_phrase(const std::string& input, const std::string& phrase) {
		//Find the position of the phrase in the string
		size_t pos = input.find(phrase);

		//If phrase is not found, return an empty string
		if (pos == std::string::npos) return "";

		//Find the starting position of the next word (skip spaces after the phrase)
		pos += phrase.length();
		while (pos < input.size() && std::isspace(input[pos]))
			++pos;

		//If we're at the end of the string, return an empty string
		if (pos == input.size()) return "";

		//Find the end position of the word (until a space or the end of the string)
		size_t endPos = pos;
		while (endPos < input.size() && !std::isspace(input[endPos]))
			++endPos;

		//Extract and return the word
		return input.substr(pos, endPos - pos);
	}
};

#endif