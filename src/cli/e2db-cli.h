/*!
 * e2-sat-editor/src/cli/e2db-cli.h
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 0.6
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <string>

using std::string;

#ifndef e2db_cli_h
#define e2db_cli_h
#include "../e2db/e2db.h"

using namespace e2se_e2db;

namespace e2se_cli
{
//TODO
class e2db_cli
{
	public:
		e2db_cli(int argc, char* argv[]);
		virtual ~e2db_cli() = default;
		int exited();

	protected:
		void options(int argc, char* argv[]);
		void version(bool verbose);
		void cmd_shell();
		void cmd_version();
		void cmd_error(string option);
		void cmd_usage(bool descriptive = false);
		void shell_exit();
		void shell_header();
		void shell_error(string is);
		void shell_command_version();
		void shell_command_help();
		void shell_command_read();
		void shell_command_write() {};
		void shell_command_list() {};
		void shell_command_add();
		void shell_command_edit() {};
		void shell_command_remove() {};
		void shell_command_set() {};
		void shell_command_unset() {};
		void shell_command_print() {};
		void shell_debugger();

		// e2db_abstract

		// e2db
		virtual void add_transponder();
		virtual void edit_transponder() {};
		virtual void remove_transponder() {};
		virtual void add_service();
		virtual void edit_service() {};
		virtual void remove_service() {};
		virtual void add_bouquet() {};
		virtual void edit_bouquet() {};
		virtual void remove_bouquet() {};
		virtual void add_userbouquet() {};
		virtual void edit_userbouquet() {};
		virtual void remove_userbouquet() {};
		virtual void add_channel_reference() {};
		virtual void edit_channel_reference() {};
		virtual void remove_channel_reference() {};
		virtual void add_tunersets() {};
		virtual void edit_tunersets() {};
		virtual void remove_tunersets() {};
		virtual void add_tunersets_table() {};
		virtual void edit_tunersets_table() {};
		virtual void remove_tunersets_table() {};
		virtual void add_tunersets_transponder() {};
		virtual void edit_tunersets_transponder() {};
		virtual void remove_tunersets_transponder() {};
		virtual void set_service_parentallock() {};
		virtual void unset_service_parentallock() {};
		virtual void set_userbouquet_parentallock() {};
		virtual void unset_userbouquet_parentallock() {};

		// e2db_parser

		// e2db_maker

		// e2db_converter

	private:
		e2db* dbih = nullptr;
};
}
#endif /* e2db_cli_h */
