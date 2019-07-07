#ifndef PFEDERC_SEMANTICS_HPP
#define PFEDERC_SEMANTICS_HPP

#include "pfederc/global.hpp"
#include "pfederc/lexer.hpp"
#include "pfederc/syntax.hpp"
#include "pfederc/utils.hpp"

namespace pfederc {
	class environment_interface;
	class environment;
	class semantic_interface;
	class semantic;
	class semantic_template;
	class semantic_trait;
	class semantic_class;
	class semantic_variable;

	class environment_interface {
		environment_interface *m_parent;
	public:
		environment_interface(environment_interface *parent = nullptr) noexcept
				: m_parent(parent) {}
		virtual ~environment_interface() {}

		/*!\return Returns optional parent
		 */
		inline auto parent() noexcept
		{ return m_parent; }

		/*!\return Returns optional parent
		 */
		inline const auto parent() const noexcept
		{ return m_parent; }

		/*!\brief Returns all for all occurences of semantic in this
		 * environment and parent environments called name.
		 * Most recent are returned first.
		 * \param name Name of the request semantic
		 */
		virtual std::vector<semantic_interface*> search(const std::string& name) const noexcept
		{ return std::vector<semantic_interface*>(); }

		/*!\brief Addes semantic to semantics of this environment.
		 * \param semantic
		 */
		virtual bool add(std::unique_ptr<semantic_interface> &&semantic) noexcept
		{ return false; }
	};

	class environment : environment_interface {
		std::vector<std::unique_ptr<semantic_interface>> m_semantics[256];
	public:
		environment() noexcept;
		virtual ~environment() {}

		virtual std::vector<semantic_interface*> search(const std::string& name) const noexcept;
		virtual bool add(std::unique_ptr<semantic_interface> &&semantic) noexcept;
	};

	class semantic_interface : environment_interface {
		std::string m_name;
	public:
		semantic_interface(const std::string &name) noexcept;
		virtual ~semantic_interface();

		/*!\return Name of the semantic with ``name().length() > 0``.
		 */
		inline const std::string& name() const noexcept
		{ return m_name; }

		/*!\return Retruns hash of name (all characters xor'ed).
		 */
		inline unsigned char hash() const noexcept {
			return std::hash<std::string>()(name()) % 256;
		}
	};

	class semantic : semantic_interface {
		environment m_env;
		std::string m_name;
	public:
		semantic(const std::string &name) noexcept;
		virtual ~semantic();

		/*!\return Name of the semantic with ``name().length() > 0``.
		 */
		inline const std::string& name() const noexcept
		{ return m_name; }

		/*!\return Retruns hash of name (all characters xor'ed).
		 */
		const unsigned char hash() const noexcept {
			return std::hash<std::string>()(name()) % 256;
		}

		virtual std::vector<semantic_interface*> search(const std::string& name) const noexcept
		{ return m_env.search(name); }
		virtual bool add(std::unique_ptr<semantic_interface> &&semantic) noexcept
		{ return m_env.add(std::move(semantic)); }
	};

	class semantic_template : public semantic_interface {
		std::vector<semantic_trait*> m_traits;
	public:
		semantic_template(const std::string &name) noexcept;
		virtual ~semantic_template();

		inline auto& implemented_traits() noexcept
		{ return m_traits; }

		inline const auto& implemented_traits() const noexcept
		{ return m_traits; }
	};

	class semantic_class : public semantic {
		environment m_objenv;
		std::vector<semantic_trait*> m_traits;
		std::vector<std::unique_ptr<semantic_template>> m_templates;
	public:
		semantic_class(const std::string &name) noexcept;
		virtual ~semantic_class();

		/*!\return Returns templates.
		 */
		inline auto& templates() noexcept
		{ return m_templates; }

		/*!\return Returns templates.
		 */
		inline const auto& templates() const noexcept
		{ return m_templates; }

		inline auto& implemented_traits() noexcept
		{ return m_traits; }

		inline const auto& implemented_traits() const noexcept
		{ return m_traits; }

		/*!\return Returns environment of objects with the type of the class
		 */
		inline auto& object_environment() noexcept
		{ return m_objenv; }

		/*!\return Returns environment of objects with the type of the class
		 */
		inline const auto& object_environment() const noexcept
		{ return m_objenv; }
	};

	class semantic_trait : public semantic_interface {
		environment m_objenv;
		std::vector<std::unique_ptr<semantic_template>> m_templates;
	public:
		semantic_trait(const std::string &name) noexcept;
		virtual ~semantic_trait();

		/*!\return Returns templates.
		 */
		inline auto& templates() noexcept
		{ return m_templates; }

		/*!\return Returns templates.
		 */
		inline const auto& templates() const noexcept
		{ return m_templates; }

		/*!\return Returns environment of objects with the type of the class
		 */
		inline environment& object_environment() noexcept
		{ return m_objenv; }

		/*!\return Returns environment of objects with the type of the class
		 */
		inline const environment& object_environment() const noexcept
		{ return m_objenv; }

		virtual bool add(std::unique_ptr<semantic_interface> &&semantic) noexcept
		{ return false; }
	};

	class semantic_variable : public semantic_interface {
		environment &m_objenv;
	public:
		semantic_variable(const std::string &name, environment &objenv) noexcept;
		virtual ~semantic_variable();

		virtual std::vector<semantic_interface*> search(const std::string& name) const noexcept
		{ return m_objenv.search(name); }

		virtual bool add(std::unique_ptr<semantic_interface> &&semantic) noexcept
		{ return false; }
	};

	class semantic_function_parameter : semantic_variable {
	public:
		semantic_variable(const std::string &name, environment &objenv) noexcept;
		virtual ~semantic_variable();
	};

	class semantic_function : public semantic_interface { 
		std::vector<std::unique_ptr<
	};
}

#endif /* PFEDERC_SEMANTICS_HPP */
