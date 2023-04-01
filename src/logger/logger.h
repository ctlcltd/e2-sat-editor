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
			bool debug = false;
		};

		inline static data* OBJECT;
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
		size_t size();
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
			this->log->debug(fn, optk, optv);
		}
		virtual void info(string fn)
		{
			this->log->info(fn);
		}
		virtual void info(string fn, string optk, string optv)
		{
			this->log->info(fn, optk, optv);
		}
		virtual void info(string fn, string optk, int optv)
		{
			this->log->info(fn, optk, optv);
		}
		virtual void error(string fn)
		{
			this->log->error(fn);
		}
		virtual void error(string fn, string optk, string optv)
		{
			this->log->error(fn, optk, optv);
		}
		virtual void error(string fn, string optk, int optv)
		{
			this->log->error(fn, optk, optv);
		}

		logger* log;
};
}
#endif /* logger_h */
