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

using std::string;

#ifndef logger_h
#define logger_h
namespace e2se
{
class logger
{
	public:
		logger(string ns);
		void debug();
		void debug(string cmsg);
		void debug(string cmsg, string optk, string optv);
		void error(string cmsg);
		void error(string cmsg, string optk, string optv);
		string ns;
};

struct log_factory
{
	protected:
		void debug(string cmsg)
		{
			this->log->debug(cmsg);
		}
		void debug(string cmsg, string optk, string optv)
		{
			this->log->debug(cmsg, optk, optv);
		}
		void error(string cmsg)
		{
			this->log->error(cmsg);
		}
		void error(string cmsg, string optk, string optv)
		{
			this->log->error(cmsg, optk, optv);
		}
		logger* log;
};
}
#endif /* logger_h */
