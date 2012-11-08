#include <string>
#include <vector>
#include <exception>
#include <stdexcept>
#include <bitset>
#include <memory>
#include <forward_list>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>

#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cassert>
#include <cerrno>

#if (defined __x86_64__ || defined __amd64__) && defined __unix__
#define RUN_MACHINE_CODE
#include <sys/mman.h>
#endif // (__x86_64__ || __amd64__) && __unix__

/* --- Tokens --------------------------------------------------------------- */

enum TokenType {TOKEN_ALNUM, TOKEN_EPS, TOKEN_DASH, TOKEN_PIPE,
		TOKEN_LT, TOKEN_GT, TOKEN_EOL, TOKEN_END};

struct Token
{
	TokenType type;                    //! Type of the token.
	unsigned line, col;                //! Location of the token within input.
	std::string value;                 //! A token string, for TOKEN_ALNUM.

	Token (unsigned line, unsigned col) : line {line}, col {col} {}

	static const char *type_to_string (TokenType type);
};

const char *
Token::type_to_string (TokenType type)
{
	switch (type)
	{
	case TOKEN_ALNUM:  return "alphanumeric";
	case TOKEN_EPS:    return "\\eps";
	case TOKEN_DASH:   return "-";
	case TOKEN_PIPE:   return "|";
	case TOKEN_LT:     return "<";
	case TOKEN_GT:     return ">";
	case TOKEN_EOL:    return "end of line";
	case TOKEN_END:    return "end of file";

	default:
		abort ();
	}
}

/* --- Scanner -------------------------------------------------------------- */

class Scanner
{
	std::istream &is;                  //! Input stream.

public:
	unsigned line, col;                //! Current position within input.

	Scanner (std::istream &is) : is (is), line {0}, col {0} {}
	Token *next ();
};

Token *
Scanner::next ()
{
	auto token = new Token (this->line + 1, this->col + 1);
	int c;

next_start:
	switch ((c = this->is.get ()))
	{
	case ' ':
	case '\t':
		this->col++;
	case '\r':
		goto next_start;

	case '-':  token->type = TOKEN_DASH;  this->col++;  return token;
	case '|':  token->type = TOKEN_PIPE;  this->col++;  return token;
	case '<':  token->type = TOKEN_LT;    this->col++;  return token;
	case '>':  token->type = TOKEN_GT;    this->col++;  return token;

	case '\n':
		token->type = TOKEN_EOL;
		this->col = 0;
		this->line++;
		return token;
	case std::istream::traits_type::eof ():
		token->type = TOKEN_END;
		return token;

	case '\\':
		if (this->is.get () != 'e'
		 || this->is.get () != 'p'
		 || this->is.get () != 's')
			goto next_fail;

		token->type = TOKEN_EPS;
		this->col += 4;
		return token;
	default:
		if (!isalnum (c))
			goto next_fail;

		do token->value += c;
		while (isalnum ((c = this->is.get ())));
		this->is.unget ();
	
		this->col += token->value.length ();

		token->type = TOKEN_ALNUM;
		return token;
	}

next_fail:
	std::cerr << "Unexpected input at "
		<< token->line << ':' << token->col << std::endl;
	delete token;
	this->is.unget ();
	return nullptr;
}

/* --- Parser --------------------------------------------------------------- */

class ParserError : public std::exception
{
	std::string s;

public:
	ParserError (std::string s) : s {s} {}
	virtual ~ParserError () noexcept {}

	virtual const char *what () const noexcept
	{
		return s.c_str ();
	}
};

class Parser
{
	std::unique_ptr<Scanner> s;        //! Where tokens come from.

public:
	std::unique_ptr<Token> token;      //! The current and the previous token.
	std::unique_ptr<Token> last;

	void error (std::string msg);
	void read ();

	Parser (Scanner *s);
	bool accept (TokenType type);
	void expect (TokenType type);
};

void
Parser::error (std::string msg)
{
	throw ParserError ("Parsing failed: " + msg);
}

void
Parser::read ()
{
	this->last.reset (this->token.release ());
	this->token.reset (this->s->next ());
	if (!this->token.get ())
		this->error ("Invalid input");
}

Parser::Parser (Scanner *s) : s {s}
{
	if (s == nullptr)
		throw std::invalid_argument ("s");

	// Could also have an init() method.
	this->read ();
}

bool
Parser::accept (TokenType type)
{
	if (this->token->type != type)
		return false;
	this->read ();
	return true;
}

void
Parser::expect (TokenType type)
{
	if (!this->accept (type))
		this->error (dynamic_cast<std::ostringstream &> (std::ostringstream ()
			<< "Expected ‘" << Token::type_to_string (type) << "’, got ‘"
			<< Token::type_to_string (this->token->type) << "’ at "
			<< this->token->line << ':' << this->token->col).str ());
}

/* --- Automata ------------------------------------------------------------- */

enum TableType {TABLE_E_NFA, TABLE_NFA, TABLE_DFA};

class Table
{
	static const unsigned max_states = 64;
	typedef std::bitset<max_states> bitset_t;

	struct State
	{
		std::string name;              //! The name of the state.
		unsigned defined  : 1;         //! Has the state been defined yet?
		unsigned is_start : 1;         //! Is this the start state?
		unsigned is_final : 1;         //! Is this a final state?
		
		std::vector<bitset_t> trans;   //! Transitions for each symbol.
	};

	std::vector<char> symbols;         //! Input symbols. 0 is for epsilon.
	std::vector<State> states;         //! All the states.
	int epsilon_index;                 //! Index of the epsilon symbol.

	void _add_state (const std::string &name);
	inline void _print_transition (const bitset_t &trans) const;
	void _epsilon_union (const Table *orig, std::vector<bitset_t> &output,
		size_t cur_state, bitset_t &visited, bool *final);
	unsigned _determinize_state (const Table *self,
		const bitset_t &state, std::vector<bitset_t> &states_masks);
	void _reduce_start_states ();
	size_t _find_algorithm_start_state () const;

	unsigned _get_state (Parser &p, const std::string &name);
	void _load_transition (Parser &p, bitset_t &trans);
	inline void _load_states (Parser &p);
	static char _load_symbol (Parser &p);
	inline void _load_alphabet (Parser &p);

public:
	TableType type;                    //! Type of the machine.

	void print () const;
	Table remove_epsilon () const;
	Table determinize () const;
	Table revert_edges () const;
	Table minimize () const;
	void to_code () const;

	static Table load (std::istream &is);
	std::basic_string<unsigned char> compile_amd64_sysv () const;
};

inline void
Table::_print_transition (const bitset_t &trans) const
{
	std::cout << ' ';
	if (trans.none ())
	{
		std::cout << '-';
		return;
	}

	bool need_pipe = false;
	for (unsigned state = 0; state < max_states; state++)
	{
		if (!trans.test (state))
			continue;

		assert (state < this->states.size ());
		if (need_pipe)
			std::cout << '|';
		std::cout << this->states[state].name;
		need_pipe = true;
	}
}

void
Table::print () const
{
	switch (this->type)
	{
	case TABLE_E_NFA:
	case TABLE_NFA:    std::cout << "NFA";  break;
	case TABLE_DFA:    std::cout << "DFA";  break;
	default:           abort ();
	}

	for (auto s : this->symbols)
		if (s)  std::cout << ' ' << s;
		else    std::cout << " \\eps";
	std::cout << std::endl;

	for (auto &s : this->states)
	{
		if (s.is_start)  std::cout << '>';
		if (s.is_final)  std::cout << '<';
		std::cout << s.name;

		for (auto &t : s.trans)
			this->_print_transition (t);
		std::cout << std::endl;
	}
}

void
Table::_epsilon_union (const Table *orig, std::vector<bitset_t> &output,
	size_t cur_state, bitset_t &visited, bool *final)
{
	assert (cur_state < this->states.size ());

	auto &trans = orig->states[cur_state].trans[orig->epsilon_index];
	for (unsigned state = 0; state < max_states; state++)
	{
		if (!trans.test (state))
			continue;

		for (size_t i = this->symbols.size (); i--; )
			output[i] |= this->states[state].trans[i];
		*final |= this->states[state].is_final;

		// Keep track of visited states to avoid infinite recursion.
		if (!visited.test (state))
		{
			visited.set (state);
			this->_epsilon_union (orig, output, state, visited, final);
		}
	}
}

Table
Table::remove_epsilon () const
{
	if (this->type != TABLE_E_NFA)
		throw std::logic_error ("Can't remove epsilon where there's none");

	assert (this->epsilon_index >= 0);
	assert ((unsigned) this->epsilon_index < this->symbols.size ());

	Table table;
	table.type = TABLE_NFA;
	table.states.resize (this->states.size ());
	table.symbols = this->symbols;
	table.symbols.erase (table.symbols.begin () + this->epsilon_index);
	table.epsilon_index = -1;

	// Copy all transitions except epsilon.
	table.states = this->states;
	for (auto &s : table.states)
		s.trans.erase (s.trans.begin () + this->epsilon_index);

	// Compute epsilon closures.
	for (size_t i = 0; i < this->states.size (); i++)
	{
		bitset_t visited;
		visited.set (i);
		bool final = table.states[i].is_final;
		table._epsilon_union (this,
			table.states[i].trans, i, visited, &final);
		table.states[i].is_final = final;
	}

	return table;
}

void
Table::_add_state (const std::string &name)
{
	if (this->states.size () == max_states)
		throw std::runtime_error ("Too many states for me");

	State s;
	s.name = name;
	s.defined = false;
	s.is_start = false;
	s.is_final = false;
	s.trans.resize (this->symbols.size ());

	this->states.push_back (s);
}

unsigned
Table::_determinize_state (const Table *self,
	const bitset_t &state, std::vector<bitset_t> &states_masks)
{
	// Similar to Table::_get_state() somewhere below.
	size_t i;
	for (i = 0; i < this->states.size (); i++)
		if (states_masks[i] == state)
			return i;

	unsigned new_state = i;
	this->_add_state (std::to_string (new_state));

	states_masks.resize (this->states.size ());
	states_masks[new_state] = state;

	auto *s = &this->states[new_state];

	// Combine the substates into it.
	for (unsigned substate = 0; substate < max_states; substate++)
	{
		if (!state.test (substate))
			continue;

		s->is_final |= self->states[substate].is_final;
		for (i = 0; i < self->symbols.size (); i++)
			s->trans[i] |= self->states[substate].trans[i];
	}

	// Translate state subsets into determinized states.
	for (i = 0; i < this->symbols.size (); i++)
	{
		if (s->trans[i].none ())
			continue;

		auto trans = this->_determinize_state
			(self, s->trans[i], states_masks);
		s = &this->states[new_state];
		s->trans[i].reset ();
		s->trans[i].set (trans);
	}

	s->defined = true;
	return new_state;
}

Table
Table::determinize () const
{
	if (this->type == TABLE_E_NFA)
		throw std::logic_error ("Can only determinize tables without epsilons");

	Table table;
	table.type = TABLE_DFA;
	table.symbols = this->symbols;
	table.epsilon_index = -1;

	// Find the start states.
	bitset_t new_start;
	for (size_t i = 0; i < this->states.size (); i++)
		if (this->states[i].is_start)
			new_start.set (i);
	assert (new_start.none () == false);

	// Scan the original table and add new numbered states.
	std::vector<bitset_t> states_masks;
	table._determinize_state (this, new_start, states_masks);

	table.states[0].is_start = true;
	return table;
}

// Not needed anymore.  It might come handy later, though...
void
Table::_reduce_start_states ()
{
	if (this->type == TABLE_E_NFA)
		throw std::logic_error ("Can't reduce start states in an epsilon NFA");

	bitset_t states;
	size_t i;
	for (i = 0; i < this->states.size (); i++)
		if (this->states[i].is_start)
			states.set (i);
	assert (states.none () == false);

	if (states.count () == 1)
		return;

	// Find a unique name for the new state.
	std::string name;
	unsigned start_id = 0;

	do
	{
		name = std::to_string (start_id++);
		for (i = 0; i < this->states.size (); i++)
			if (this->states[i].name == name)
				break;
	}
	while (i != this->states.size ());

	// Allocate the entry.
	unsigned new_state = i;
	this->_add_state (name);

	// Make the new state go on epsilon to all of the original
	// start states; only without any actual epsilon involved.
	auto &s = this->states[new_state];
	s.defined  = true;
	s.is_start = true;

	for (unsigned m = 0; m < max_states; m++)
	{
		if (!states.test (m))
			continue;
		this->states[m].is_start = false;
		this->states[new_state].is_final |= this->states[m].is_final;

		for (size_t k = this->symbols.size (); k--; )
			this->states[new_state].trans[k] |=
				this->states[m].trans[k];
	}
}

Table
Table::revert_edges () const
{
	if (this->type == TABLE_E_NFA)
		throw std::logic_error ("Can't revert edges in an epsilon NFA");

	Table table;
	table.type = TABLE_NFA;
	table.symbols = this->symbols;
	table.states.resize (this->states.size ());
	table.epsilon_index = -1;

	// Initialize an empty copy of the transition table.
	for (size_t i = this->states.size (); i--; )
	{
		auto *s_new = &table.states[i];
		auto *s_old = &this->states[i];

		s_new->name     = s_old->name;
		s_new->defined  = s_old->defined;
		s_new->is_final = s_old->is_start;
		s_new->is_start = s_old->is_final;

		s_new->trans.resize (table.symbols.size ());
	}

	// Revert edges.
	for (size_t k = this->symbols.size (); k--; )
		for (size_t i = this->states.size (); i--; )
	{
		auto &state = this->states[i].trans[k];
		if (state.none ())
			continue;

		for (unsigned m = max_states; m--; )
			if (state.test (m))
				table.states[m].trans[k].set (i);
	}

	return table;
}

Table
Table::minimize () const
{
	if (this->type == TABLE_E_NFA)
		throw std::logic_error ("Can't minimize an epsilon NFA");

	// Brzozowski's algorithm:
	return this->revert_edges ().determinize ().revert_edges ().determinize ();
}

size_t
Table::_find_algorithm_start_state () const
{
	if (this->type != TABLE_DFA)
		throw std::logic_error ("Can only produce an algorithm for a DFA");

	bool found_start = false;
	size_t start_state = 0;
	for (size_t i = this->states.size (); i--; )
	{
		if (!this->states[i].is_start)
			continue;

		if (found_start)
			throw std::logic_error ("Can't produce an algorithm "
				"for multiple start states");

		start_state = i;
		found_start = true;
	}

	assert (found_start == true);
	return start_state;
}

void
Table::to_code () const
{
	size_t i, start_state = this->_find_algorithm_start_state ();

	std::cout << "bool checkString (const char *s) {\n"
		"  #define ACCEPT case 0: return true;\n"
		"  #define END default: return false; }\n"
#ifdef USE_GOTO
		"  #define S(a) a: switch (*s++) {\n"
		"  #define T(a,b) case a: goto b;\n\n";

	for (size_t xi = 0; xi < this->states.size (); xi++)
	{
		if (!xi)  i = start_state;
		else      i = (xi == start_state) ? 0 : xi;
#else // ! USE_GOTO
		"  #define BEGIN while (true) switch (q) {\n"
		"  #define S(a) case a: switch (*s++) {\n"
		"  #define T(a,b) case a: q = b; continue;\n\n"
		"  enum {";

	bool need_comma = false;
	for (i = 0; i < this->states.size (); i++)
	{
		if (need_comma)
			std::cout << ((i != 0 && i % 10 == 0) ? ",\n    " : ", ");
		std::cout << 'S' << this->states[i].name;
		need_comma = true;
	}

	std::cout << "} q = S" << this->states[start_state].name
		<< ";\n\n  BEGIN\n";

	for (i = 0; i < this->states.size (); i++)
	{
#endif // ! USE_GOTO
		std::cout << "  S(S" << this->states[i].name << ')';

		for (size_t k = 0; k < this->symbols.size (); k++)
		{
			auto &b = this->states[i].trans[k];
			if (b.none ())
				continue;

			unsigned state = max_states - 1;
			while (!b.test (state))
				state--;

			std::cout << " T('" << this->symbols[k]
				<< "',S" << this->states[state].name << ')';
		}

		if (this->states[i].is_final)
			std::cout << " ACCEPT";
		std::cout << " END\n";
	}

#ifndef USE_GOTO
	std::cout << "  END\n";
#endif // ! USE_GOTO
	std::cout << "}" << std::endl;
}

/* --- Automata - Loading --------------------------------------------------- */

char
Table::_load_symbol (Parser &p)
{
	if (p.accept (TOKEN_EPS))
		return 0;
	else
	{
		p.expect (TOKEN_ALNUM);
		if (p.last->value.length () != 1)
			p.error ("Input symbols must be of length 1: " + p.last->value);
		return p.last->value[0];
	}
}

inline void
Table::_load_alphabet (Parser &p)
{
	this->epsilon_index = -1;

	do
	{
		char symbol = Table::_load_symbol (p);
		for (auto s : this->symbols)
			if (s == symbol)
				p.error ("Can't have repeating input symbols: " + symbol);
		if (!symbol)
			this->epsilon_index = this->symbols.size ();
		this->symbols.push_back (symbol);
	}
	while (!p.accept (TOKEN_EOL));

	if (this->epsilon_index != -1)
	{
		if (this->type == TABLE_DFA)
			p.error ("Can't have DFA's with epsilon transitions");
		this->type = TABLE_E_NFA;
	}
}

unsigned
Table::_get_state (Parser &p, const std::string &name)
{
	size_t i;
	for (i = 0; i < this->states.size (); i++)
		if (this->states[i].name == name)
			return i;

	try
	{
		this->_add_state (name);
	}
	catch (const std::exception &e)
	{
		p.error (e.what ());
	}
	return i;
}

void
Table::_load_transition (Parser &p, bitset_t &trans)
{
	if (p.accept (TOKEN_DASH))
		return;

	do
	{
		p.expect (TOKEN_ALNUM);
		trans.set (this->_get_state (p, p.last->value));
	}
	while (this->type != TABLE_DFA && p.accept (TOKEN_PIPE));
}

inline void
Table::_load_states (Parser &p)
{
	bool have_start = false;
	bool have_final = false;

	while (!p.accept (TOKEN_END))
	{
		bool is_start = false;
		if (p.accept (TOKEN_GT))
			have_start = is_start = true;

		bool is_final = false;
		if (p.accept (TOKEN_LT))
			have_final = is_final = true;

		p.expect (TOKEN_ALNUM);
		unsigned state = this->_get_state (p, p.last->value);
		auto *s = &this->states[state];

		if (s->defined)
			p.error ("Redefining state " + p.last->value
				+ " on line " + std::to_string (p.last->line));

		s->defined = true;
		s->is_start = is_start;
		s->is_final = is_final;

		for (size_t i = 0; i < this->symbols.size (); i++)
		{
			this->_load_transition (p, s->trans[i]);
			s = &this->states[state];
		}

		if (!p.accept (TOKEN_END))
			p.expect (TOKEN_EOL);
	}

	if (!this->states.size ())  p.error ("No states");
	if (!have_start)            p.error ("No start state");
	if (!have_final)            p.error ("No final state");

	for (auto &s : this->states)
		if (!s.defined)
			p.error ("Undefined state: " + s.name);
}

Table
Table::load (std::istream &is)
{
	Parser p (new Scanner (is));
	Table self;

	p.expect (TOKEN_ALNUM);
	if      (p.last->value == "NFA")  self.type = TABLE_NFA;
	else if (p.last->value == "DFA")  self.type = TABLE_DFA;
	else     p.error ("Invalid table type: " + p.last->value);

	self._load_alphabet (p);
	self._load_states (p);

	return self;
}

/* --- Compiling ------------------------------------------------------------ */

struct AddressLink
{
	unsigned state;                    // ID of the state to jump to.
	unsigned offset;                   // Offset to the address within code.
};

std::basic_string<unsigned char>
Table::compile_amd64_sysv () const
{
	size_t i, start_state = this->_find_algorithm_start_state ();

	static const unsigned char
		xstate [] = "\x0F\xB6\x07"               // movzx eax, byte [rdi]
		            "\x48\x83\xC7\x01",          // add rdi, byte 1
		xinput [] = "\x3C" /* id */,             // cmp al, <id>
		xdelta [] = "\x0F\x84\x00\x00\x00\x00",  // jz <addr32>
		xaccept[] = "\x84\xC0"                   // test al, al
		            "\x0F\x94\xC0"               // setz al
		            "\x0F\xB6\xC0",              // movzx eax, al
		xreject[] = "\x31\xC0",                  // xor eax, eax
		xreturn[] = "\xC3";                      // ret

	std::basic_string<unsigned char> code;
	#define APPEND_CODE(x)  code.append (x, sizeof x - 1)

	unsigned state_offsets[this->states.size ()];
	std::forward_list<AddressLink> fixups;

	for (size_t xi = 0; xi < this->states.size (); xi++)
	{
		if (!xi)  i = start_state;
		else      i = (xi == start_state) ? 0 : xi;

		state_offsets[i] = code.size ();
		APPEND_CODE (xstate);

		for (size_t k = 0; k < this->symbols.size (); k++)
		{
			auto &b = this->states[i].trans[k];
			if (b.none ())
				continue;

			unsigned state = max_states - 1;
			while (!b.test (state))
				state--;

			APPEND_CODE (xinput);
			code += this->symbols[k];
			APPEND_CODE (xdelta);

			AddressLink link;
			link.state = state;
			link.offset = code.size () - 4;
			fixups.push_front (link);
		}

		if (this->states[i].is_final)
			APPEND_CODE (xaccept);
		else
			APPEND_CODE (xreject);
		APPEND_CODE (xreturn);
	}

	// Fix relative jumps to other states.
	for (auto &iter : fixups)
	{
		int32_t rel32
			= (int32_t) state_offsets[iter.state]
			- (int32_t) (iter.offset + 4);

		code[iter.offset]     =  rel32        & 0xFF;
		code[iter.offset + 1] = (rel32 >> 8)  & 0xFF;
		code[iter.offset + 2] = (rel32 >> 16) & 0xFF;
		code[iter.offset + 3] = (rel32 >> 24) & 0xFF;
	}

	return code;
}

/* --- Processing ----------------------------------------------------------- */

static void
terminate_handler ()
{
	try
	{
		std::exception_ptr pe = std::current_exception ();
		if (pe != std::exception_ptr ())
			std::rethrow_exception (pe);
	}
	catch (const std::exception &e)
	{
		std::cerr << "Unhandled error: " << e.what () << std::endl;
	}

	exit (EXIT_FAILURE);
}

static inline void
create_machine_code (const Table &table)
{
	auto compiled = table.compile_amd64_sysv ();

	std::cout << "/* amd64 machine code, System V calling convention */\n"
		"const char *code = \"";
	std::cout << std::hex << std::setfill ('0');

	for (unsigned i = 0; i < compiled.size (); i++)
	{
		if (i != 0 && !(i % 16))
			std::cout << "\"\n\t\"";
		std::cout << "\\x" << std::setw (2) << (int) compiled[i];
	}

	std::cout.copyfmt (std::ios (nullptr));
	std::cout << "\";\n" << std::endl;

#ifdef RUN_MACHINE_CODE
	if (std::cin.eof ())
		return;

	auto executable = mmap (NULL, compiled.size (),
		PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (!executable)
	{
		std::cerr << "Error running the machine: mmap: "
			<< strerror (errno) << std::endl;
		return;
	}

	memcpy (executable, compiled.data (), compiled.size ());

	std::cout << "Here you can test inputs on the machine. "
		"Press ^D to terminate.\n";
	std::string s;
	std::cout << "> " << std::flush;
	while (getline (std::cin, s))
		std::cout <<  (((uint32_t (*) (const char *)) executable) (s.c_str ())
			? "< accepted" : "< rejected") << "\n> " << std::flush;

	munmap (executable, compiled.size ());
	std::cout << std::endl << std::endl;
#endif // RUN_MACHINE_CODE
}

static void
transform (const char *msg, Table *ptable, Table (Table::*fun) () const)
{
	std::cout << msg << std::endl;
	(*ptable = (ptable->*fun) ()).print ();
	std::cout << std::endl;
}

int
main (int argc, char *argv[])
{
	std::set_terminate (terminate_handler);

	if (argc > 2)
	{
		std::cerr << "Usage: " << argv[0] << " [input-file]" << std::endl;
		exit (EXIT_FAILURE);
	}

	Table table;
	if (argc == 2)
	{
		auto file = argv[1];
		std::ifstream input (file);
		if (!input)
		{
			std::cerr << "Error opening " << file << std::endl;
			exit (EXIT_FAILURE);
		}
		table = Table::load (input);
	}
	else
		table = Table::load (std::cin);

	if (table.type == TABLE_E_NFA)
		transform ("Removing epsilon transitions...",
			&table, &Table::remove_epsilon);
	if (table.type == TABLE_NFA)
		transform ("Determinizing the NFA...", &table, &Table::determinize);

	transform ("Minimizing the DFA...", &table, &Table::minimize);

	table.to_code ();
	std::cout << std::endl;

	create_machine_code (table);

	transform ("Forming an NFA for the reversed language...",
		&table, &Table::revert_edges);

	return 0;
}

