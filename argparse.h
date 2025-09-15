#ifndef __ARGPARSE_H__
#define __ARGPARSE_H__


#include <set>
#include <memory>
#include <string>
#include <vector>
#include <utility>
#include <unordered_map>


class ITokenParser {
	public:
		virtual bool parse(const std::string& input) = 0;
};


class ArgumentParser {
	public:
		ArgumentParser(const std::string& program_name, const std::string& program_description);

		bool add_flag(const std::string& argument_name, bool& ref);

		template <typename T>
		bool add_option(const std::string& argument_name, T& ref);

		template <typename T>
		bool add_positional(const std::string& argument_name, T& ref);

		bool parse_args(int argc, char* argv[]);

	private:
		const std::string m_program_name;
		const std::string m_program_description;

		std::unordered_map<std::string, bool&> m_flags_dict;
		std::unordered_map<std::string, std::unique_ptr<ITokenParser>> m_optionals_dict;

		// positional arguments don't need a lookup.
		// this set only handles name collisions
		std::set<std::string> m_positional_dict;
		std::vector<std::unique_ptr<ITokenParser>> m_positional_vect;
};


#ifdef ARGPARSE_IMPLEMENTATION
#include <regex>
#include <format>
#include <iostream>


const std::regex flag_rule = std::regex("-[\-a-zA-Z]+");
const std::regex optional_rule = flag_rule;
const std::regex positional_rule = std::regex("[_a-zA-Z][_a-zA-Z0-9]*");


// we could play with the return type. It could be of type T, or
// a string describing the error message
template <typename T>
struct ParseTraits {
	static std::optional<T> parse(const std::string& s);
};


template <>
struct ParseTraits<int> {
	static std::optional<int> parse(const std::string& s) {
		try {
			return std::stoi(s);
		} catch(std::exception const& e) {
			return std::nullopt;
		};
	}
};


template <>
struct ParseTraits<double> {
	static std::optional<int> parse(const std::string& s) {
		try {
			return std::stod(s);
		} catch(std::exception const& e) {
			return std::nullopt;
		};
	}
};


template <>
struct ParseTraits<std::string> {
	static std::optional<std::string> parse(const std::string &s) {
		return s;
	}
};


template <typename T>
class TokenParser : public ITokenParser {
	public:
		TokenParser(T& ref) : m_ref(ref) {};

		bool parse(const std::string& input) override {
			const auto result = ParseTraits<T>::parse(input);

			if (result) {
				m_ref = *result;
				return true;
			}

			return false;
		}

	private:
		T& m_ref;
};


ArgumentParser::ArgumentParser(const std::string& program_name, const std::string& program_description) :
	m_program_name(program_name),
	m_program_description(program_description)
{}


bool ArgumentParser::add_flag(const std::string& argument_name, bool& ref) {
	if (not std::regex_match(argument_name, flag_rule)) {
		std::cerr << std::format("[ERROR]: `{}` is not an appropriate flag name\n", argument_name);
		return false;
	}

	if (m_flags_dict.contains(argument_name) or m_optionals_dict.contains(argument_name)) {
		std::cerr << std::format("[ERROR]: `{}` has been already registered\n", argument_name);
		return false;
	}

	m_flags_dict[argument_name] = ref;
}


template <typename T>
bool ArgumentParser::add_option(const std::string& argument_name, T& ref) {
	if (not std::regex_match(argument_name, option_rule)) {
		std::cerr << std::format("[ERROR]: `{}` is not an appropriate option name\n", argument_name);
		return false;
	}

	if (m_flags_dict.contains(argument_name) or m_optionals_dict.contains(argument_name)) {
		std::cerr << std::format("[ERROR]: `{}` has been already registered\n", argument_name);
		return false;
	}

	// TODO: add parser to dictionary
}


template <typename T>
bool ArgumentParser::add_positional(const std::string& argument_name, T& ref) {
	if (nor std::regex_match(argument_name, positional_rule)) {
		std::cerr << std::format("[ERROR]: `{}` is not an appropriate positional name, must be an identifier\n", argument_name);
		return false;
	}

	if (m_positional_dict.contains(argument_name)) {
		std::cerr << std::format("[ERROR]: another positional with name `{}` has already been registered\n", argument_name);
		return false;
	}

	m_positional_dict.insert(argument_name);

	// TODO: add parser to vector of parsers
}


bool ArgumentParser::parse_args(int argc, char* argv[]) {
	return false;
}


#endif // ARGPARSE_IMPLEMENTATION
#endif // __ARGPARSE_H__
