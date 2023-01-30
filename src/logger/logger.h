/*!
 * e2-sat-editor/src/logger/logger.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.3
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <string>
#include <sstream>

using std::string;

#ifndef logger_h
#define logger_h
namespace e2se
{
class logger
{
	public:
		struct data {
			string log;
			size_t size;
			bool debug;
		};

		static data* OBJECT;
		inline static string PREFIX = "e2se";

		logger(string ns, string cn);
		logger(data* obj, string ns, string cn);
		void debug(string fn);
		void debug(string fn, string optk, string optv);
		void debug(string fn, string optk, int optv);
		void info(string fn);
		void info(string fn, string optk, string optv);
		void info(string fn, string optk, int optv);
		void error(string fn);
		void error(string fn, string optk, string optv);
		void error(string fn, string optk, int optv);
		string timestamp();
		string str();
		string str_lend();
		size_t pos();
		size_t last_pos();
		std::stringbuf* buf;
		string ns;
		string cn;
		data* obj;
};

struct log_factory
{
	protected:
		void debug(string fn)
		{
			this->log->debug(fn);
		}
		void debug(string fn, string optk, string optv)
		{
			this->log->debug(fn, optk, optv);
		}
		void debug(string fn, string optk, int optv)
		{
			this->log->debug(fn, optk, optv);
		}
		void info(string fn)
		{
			this->log->info(fn);
		}
		void info(string fn, string optk, string optv)
		{
			this->log->info(fn, optk, optv);
		}
		void info(string fn, string optk, int optv)
		{
			this->log->info(fn, optk, optv);
		}
		void error(string fn)
		{
			this->log->error(fn);
		}
		void error(string fn, string optk, string optv)
		{
			this->log->error(fn, optk, optv);
		}
		void error(string fn, string optk, int optv)
		{
			this->log->error(fn, optk, optv);
		}

		logger* log;
};
}
#endif /* logger_h */
