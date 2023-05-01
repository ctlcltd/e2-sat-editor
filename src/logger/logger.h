/*!
 * e2-sat-editor/src/logger/logger.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.6
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <string>
#include <sstream>
#include <iostream>
#include <stdexcept>

using std::string;

#ifndef logger_h
#define logger_h
namespace e2se
{
class logger
{
	public:
		enum MSG {
			except_uncaught,
			except_invalid_argument,
			except_out_of_range,
			except_bad_any_cast,
			except_filesystem
		};

		struct data
		{
			string log;
			bool debug = false;
			bool cli = false;
		};

		inline static data* OBJECT;
		inline static string PREFIX = "e2se";

		logger(string ns, string cn);
		logger(data* obj, string ns, string cn);
		void debug(string fn);
		void debug(string fn, string optk, string optv);
		void info(string fn, string optk, string optv);
		void error(string fn, string optk, string optv);
		string timestamp();
		string str();
		size_t size();
		static string msg(string str, string param);
		static string msg(string str);
		static string msg(MSG msg, const char* param);
		static string msg(MSG msg);
		std::stringbuf* buf;
		string ns;
		string cn;
		data* obj;
};

struct log_factory
{
	protected:
		virtual void debug(string fn)
		{
			this->log->debug(fn);
		}
		virtual void debug(string fn, string optk, string optv)
		{
			this->log->debug(fn, optk, optv);
		}
		virtual void debug(string fn, string optk, int optv)
		{
			this->log->debug(fn, optk, std::to_string(optv));
		}
		virtual void info(string fn, string optk, string optv)
		{
			this->log->info(fn, optk, optv);

			if (this->log->obj->cli)
				std::cout << "Info: " << optk << ": " << optv << std::endl;
		}
		virtual void error(string fn, string optk, string optv)
		{
			this->log->error(fn, optk, optv);

			if (this->log->obj->cli)
				throw std::runtime_error (optk + ": " + optv);
		}
		virtual void exception(string fn, string optk, string optv)
		{
			this->log->error(fn, optk, optv);

			if (this->log->obj->cli)
				throw;
		}

		logger* log;
};
}
#endif /* logger_h */
