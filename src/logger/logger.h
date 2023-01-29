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

		logger(string ns);
		logger(data* obj, string ns);
		void debug(string msg);
		void debug(string msg, string optk, string optv);
		void debug(string msg, string optk, int optv);
		void info(string msg);
		void info(string msg, string optk, string optv);
		void info(string msg, string optk, int optv);
		void error(string msg);
		void error(string msg, string optk, string optv);
		void error(string msg, string optk, int optv);
		string timestamp();
		string str();
		string str_lend();
		size_t pos();
		size_t last_pos();
		std::stringbuf* buf;
		string ns;
		data* obj;
};

struct log_factory
{
	public:
		logger* log;

	protected:
		void debug(string msg)
		{
			this->log->debug(msg);
		}
		void debug(string msg, string optk, string optv)
		{
			this->log->debug(msg, optk, optv);
		}
		void debug(string msg, string optk, int optv)
		{
			this->log->debug(msg, optk, optv);
		}
		void info(string msg)
		{
			this->log->info(msg);
		}
		void info(string msg, string optk, string optv)
		{
			this->log->info(msg, optk, optv);
		}
		void info(string msg, string optk, int optv)
		{
			this->log->info(msg, optk, optv);
		}
		void error(string msg)
		{
			this->log->error(msg);
		}
		void error(string msg, string optk, string optv)
		{
			this->log->error(msg, optk, optv);
		}
		void error(string msg, string optk, int optv)
		{
			this->log->error(msg, optk, optv);
		}
};
}
#endif /* logger_h */
