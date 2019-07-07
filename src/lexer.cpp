#include "pfederc/lexer.hpp"

/*******************************************************************************
 * pos
 ******************************************************************************/

pfederc::pos pfederc::operator +(const pfederc::pos& p0,
		       const pfederc::pos& p1) noexcept {
	return pfederc::pos{
		std::min(p0.column_min, p1.column_min),
		std::max(p0.column_max, p1.column_max),
		std::min(p0.line_min, p1.line_min),
		std::max(p0.line_max, p1.line_max)};
}

/*******************************************************************************
 * token
 ******************************************************************************/

static std::string _to_feder_str(const std::string &str) {
	std::string result;
	for (size_t i = 0; i < str.length(); ++i) {
		switch (str[i]) {
			case '\0': result += "\\0"; break;
			case '\b': result += "\\b"; break;
			case '\a': result += "\\a"; break;
			case '\f': result += "\\f"; break;
			case '\n': result += "\\n"; break;
			case '\r': result += "\\r"; break;
			case '\t': result += "\\t"; break;
			case '\v': result += "\\v"; break;
			case '\\': result += "\\\\"; break;
			case '\'': result += "\\\'"; break;
			case '\"': result += "\\\""; break;
			default:
				result += str[i];
		}
	}

	return result;
}

std::string pfederc::token::to_string() const noexcept {
	switch(this->type()) {
	case pfederc::token_type::id:
		return this->str();
	// constant values
	case pfederc::token_type::num:
		switch (this->number_type()) {
		case pfederc::num_type::i8:
			return std::to_string(this->number_value().i8) + "s";
		case pfederc::num_type::u8:
			return std::to_string(this->number_value().u8) + "us";
		case pfederc::num_type::i16:
			return std::to_string(this->number_value().i16) + "S";
		case pfederc::num_type::u16:
			return std::to_string(this->number_value().u16) + "uS";
		case pfederc::num_type::i32:
			return std::to_string(this->number_value().i32);
		case pfederc::num_type::u32:
			return std::to_string(this->number_value().u32) + "u";
		case pfederc::num_type::i64:
			return std::to_string(this->number_value().i64);
		case pfederc::num_type::u64:
			return std::to_string(this->number_value().u64) + "u";

		case pfederc::num_type::f32:
			return std::to_string(this->number_value().f32);
		case pfederc::num_type::f64:
			return std::to_string(this->number_value().f64) + "F";
		}
	case pfederc::token_type::str:
		return "\"" + _to_feder_str(this->str()) + "\"";
	case pfederc::token_type::character:
		return "\'" + _to_feder_str(this->str()) + "\'";
	// constrol signs
	case pfederc::token_type::newline: {
		std::string result;
		std::size_t i = position().line_max - position().line_min + 1;
		while (i-- > 0) {
			result += '\n';
		}
		return result;
	}
	case pfederc::token_type::eof:
		return std::string();
	// error
	case pfederc::token_type::err:
		return this->str();
	default:
		PFEDERC_PANIC("Unexpected lexical token.");
		return std::string(); // the compiler shouldn't worry too much
	}
}

/*******************************************************************************
 * tokenizer
 ******************************************************************************/

pfederc::tokenizer::tokenizer(std::istream &input) noexcept
		: m_input{input}, curpos{0,0,0,0}, curchar{'\0'},
      m_tokcur{NULL} {
}

void pfederc::tokenizer::_next() {
	// skip spaces
	while (curchar == ' '
			|| curchar == '\t'
			|| curchar == '\v')
		curchar = m_input.get();

	switch (curchar) {
		// newline
		case '\n':
			curpos.column_min = curpos.column_max = 0;
			++curpos.line_max;
			// eat carriage-return
			if (curchar == '\r')
				curchar = m_input.get();
			// read consecutive lines
			while (curchar == '\n') {
				++curpos.line_max;
				curchar = m_input.get();
				if (curchar == '\r')
					curchar = m_input.get();
			}
			m_tokcur->type() = pfederc::token_type::newline;
			return;
		case '\r':
			curpos.column_min = curpos.column_max = 0;
			++curpos.line_max;
			// eat carriage-return
			if (curchar == '\n')
				curchar = m_input.get();
			// read consecutive lines
			while (curchar == '\r') {
				++curpos.line_max;
				curchar = m_input.get();
				if (curchar == '\n')
					curchar = m_input.get();
			}
			m_tokcur->type() = pfederc::token_type::newline;
			return;
		// eof
		case -1:
			m_tokcur->type() = pfederc::token_type::eof;
			return;
	}
}
