/*!
 * e2-sat-editor/src/logger/logger.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.1
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <string>
#include <iostream>
#include <sstream>

using std::string;

#ifndef logger_h
#define logger_h
namespace e2se
{
class logger
{
	public:
		struct session {
			string text;
			bool debug;
		};
		logger(string ns);
		logger(session* log, string ns);
		void debug(string msg);
		void debug(string msg, string optk, string optv);
		void debug(string msg, string optk, int optv);
		void debug(string msg, string optk, bool optv);
		void error(string msg);
		void error(string msg, string optk, string optv);
		string str();
		std::stringbuf* buf;
		string ns;
		session* log;
};

struct log_factory
{
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
		void debug(string msg, string optk, bool optv)
		{
			this->log->debug(msg, optk, optv);
		}
		void error(string msg)
		{
			this->log->error(msg);
		}
		void error(string msg, string optk, string optv)
		{
			this->log->error(msg, optk, optv);
		}
		logger* log;
};
}
#endif /* logger_h */
