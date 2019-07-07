#ifndef PFEDERC_LEXER_HPP
#define PFEDERC_LEXER_HPP

#include "pfederc/global.hpp"

namespace pfederc {
	enum token_type : uint16_t {
		// identifiers
		id,
		// operators
		key, op,
		// constants
		num, str, character,
		// command sep.
		newline, eof,
		// error
		err
	};

	enum num_type : uint8_t {
		// mask for unsigned
		u = 0x80,
    // integers
		i8 = 0x00,  u8 = 0x80,
		i16 = 0x01, u16 = 0x81,
		i32 = 0x02, u32 = 0x82,
		i64 = 0x03, u64 = 0x83,
		// floating-point
		f32 = 0x0A, f64 = 0x0B
	};

	union num_value {
		// integer
		signed char i8;
		unsigned char u8;
		signed short i16;
		unsigned char u16;
		signed int i32;
		unsigned int u32;
		signed long int i64;
		unsigned long int u64;
		// floating-point
		float f32;
		double f64;
	};

	class token;
	class tokenizer;

	struct pos {
		std::size_t column_min, //!< colunm_min <= column_max
			          column_max; //!< column_max >= column_min
		std::size_t line_min,   //!< ``line_min <= line_max``
			          line_max;   //!< ``line_max >= line_min``
	};

	/*!\brief Merges p0, p1.
	 * \param p0
	 * \param p1
	 */
	pos operator +(const pos& p0, const pos& p1) noexcept;

	/*****************************************************************************
	 * token
	 ****************************************************************************/

	class token {
		std::string m_str;
		token_type m_type;
		num_type m_num_type;
		num_value m_num_val;

		token *m_prev, *m_next;
		pos m_pos;

		std::size_t m_id;
	public:
		token(token *prev, std::size_t id,
				pos tokpos, token_type m_type)
			: m_prev{prev}, m_next{nullptr},
		    m_id{id},
				m_pos(tokpos),
				m_str(), m_num_type{num_type::u64}, m_num_val{0L} {}

		virtual ~token()
		{ if(m_prev) { delete m_prev; m_prev = nullptr; } }

		/*!\return Lexer token identifier.
		 */
		std::size_t id() const noexcept { return m_id; }

		/*!\return Returns code position.
		 */
		inline pos& position() noexcept { return m_pos; }

		/*!\return Returns code position.
		 */
		inline const pos& position() const noexcept { return m_pos; }

		/*!\return Returns number type.
		 */
		inline num_type& number_type() noexcept { return m_num_type; }

		/*!\return Returns number type.
		 */
		inline const num_type number_type() const noexcept { return m_num_type; }

		/*!\return Returns number value.
		 */
		inline num_value& number_value() noexcept { return m_num_val; }

		/*!\return Returns number value.
		 */
		inline const num_value number_value() const noexcept { return m_num_val; }

		/*!\return Returns optional associated string.
		 */
		inline std::string& str() noexcept { return m_str; }

		/*!\return Returns optional associated string.
		 */
		inline const std::string& str() const noexcept { return m_str; }

		/*!\return returns token type
		 */
		inline token_type& type() noexcept { return m_type; }

		/*!\return returns token type
		 */
		inline token_type type() const noexcept { return m_type; }

		/*!\return Returns optional previous token.
		 */
		inline token& operator --() noexcept { return *m_prev; }

		/*!\return Returns optional previous token.
		 */
		inline const token& operator --() const noexcept { return *m_prev; }

		/*!\return Returns optional next token.
		 */
		inline token& operator ++() noexcept { return *m_next; }

		/*!\return Returns optional next token.
		 */
		inline const token& operator ++() const noexcept { return *m_next; }

		std::string to_string() const noexcept;
	};

	struct tokenizer_props {
		bool utf8_id; //!< UTF8 identifiers
		unsigned char tabsize; //!< Size of tabulator in spaces
	};

	//! Default tokenizer properties
	constexpr tokenizer_props TOKENIZER_DEFAULT_PROPS =
		tokenizer_props{false, 2};

	/*****************************************************************************
	 * tokenizer
	 ****************************************************************************/

	class tokenizer {
		std::istream &m_input;

		pos curpos;
		token *m_tokcur;

		int curchar;

		void _next();
	public:
		tokenizer(std::istream &input) noexcept;
		virtual ~tokenizer();

		const token *next() {
			if (!m_tokcur)
				curchar = m_input.get();

			m_tokcur = new token(m_tokcur,
					m_tokcur ? m_tokcur->id() + 1 : 0,
					curpos, pfederc::token_type::err);
			_next();
			m_tokcur->position() = curpos; // set to current, was copied before
			return m_tokcur;
		}

		inline const token *current() const {
			if (!m_tokcur)
				throw std::runtime_error("Excepted pfederc::tokenizer.next call.");

			return m_tokcur;
		}
	};
}

#endif /* PFEDERC_LEXER_HPP */
