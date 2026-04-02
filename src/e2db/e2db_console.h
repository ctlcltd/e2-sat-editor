/*!
 * e2-sat-editor/src/e2db/e2db_console.h
 * 
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.9.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include <iostream>
#include <string>
#include <vector>
#include <any>

using std::string, std::vector;

#ifndef e2db_console_h
#define e2db_console_h
#include "../logger/logger.h"
#include "../e2db/e2db.h"

namespace e2se_e2db
{

struct termiface
{
	public:
		virtual ~termiface() = default;
		static void reset() {}
		virtual void input(bool shell = false, bool ins = false) = 0;
		virtual void clear() = 0;
		virtual const std::string str() = 0;
		virtual std::istream* stream() = 0;
		static int paged(int pos, int offset) { return 0; }
		static std::pair<int, int> screensize() { return {}; }
		virtual void dump_log() = 0;
		virtual void load_history() = 0;
		virtual void save_history() = 0;
};

class e2db_console
{
	public:
		virtual ~e2db_console() = default;
		int exited();

	protected:

		static const int PAGED_LIMIT = 10;

		enum COMMAND {
			usage,
			fread,
			fwrite,
			list,
			add,
			edit,
			remove,
			copy,
			move,
			set,
			unset,
			fimport,
			fexport,
			merge,
			print,
			parse,
			make,
			convert,
			tool,
			macro,
			debug,
			preferences
		};

		enum ENTRY {
			index,
			all,
			lamedb,
			zapit,
			transponder,
			service,
			lamedb_services = service,
			zapit_services,
			bouquet,
			zapit_bouquets,
			userbouquet,
			channel_reference,
			tunersets,
			tunersets_table,
			tunersets_transponder,
			parentallock,
			parentallock_blacklist,
			parentallock_whitelist,
			parentallock_locked
		};

		enum TYPE {
			dbtype, dbparental, idx, 
			chid, txid, refid, tvid, tnid, trid, yname, ytype, 
			ssid, dvbns, tsid, onid, stype, snum, srcid, parental, chname, 
			sdata_p, sdata_c, sdata_C, sdata_f, 
			freq, sr, pol, fec, hpfec, lpfec, cfec, inv, tinv, cinv, sys, tsys, csys, asys, mod, tmod, cmod, amod, rol, pil, band, tmx, guard, hier, plpid, 
			chdata, txdata, ffdata, bsdata, ubdata, tvdata, tndata, flags, 
			isid, plscode, plsmode, t2mi_plpid, t2mi_pid, mts, plsn, 
			pos, diseqc, uncomtd, charset, 
			tname, country, feed, 
			bname, pname, rname, qname, nname, btype, hidden, locked, 
			mname, fname, itype, 
			churl, chvalue, etype, atype, mnum, 
		};

		enum VALUE {
			val_int,
			val_char,
			val_bool,
			val_string,
			val_obj
		};

		enum ESCAPE {
			name_begin,
			name_end,
			divider,
			value_begin,
			value_end
		};

		enum OBJIO {
			_std = -1,
			tabular = 0,
			byline = 1,
			json = 2
		};

		enum HISTORY {
			file = 0,
			memory = 1
		};

		struct iosets
		{
			OBJIO in = OBJIO::_std;
			OBJIO out = OBJIO::tabular;
			bool hrn = true; // human readable pair name
			bool hrv = true; // human readable pair value
			bool ppg = true; // paged preamble
		} __objio;

		HISTORY history = HISTORY::memory;

		void version(bool verbose);
		void console_exit();
		void console_header();
		void console_error(const string& cmd);
		void command_version();
		void command_help(istream* is) { console_resolver(COMMAND::usage, is); }
		void command_read(istream* is) { console_resolver(COMMAND::fread, is); }
		void command_write(istream* is) { console_resolver(COMMAND::fwrite, is); }
		void command_list(istream* is) { console_resolver(COMMAND::list, is); }
		void command_add(istream* is) { console_resolver(COMMAND::add, is); }
		void command_edit(istream* is) { console_resolver(COMMAND::edit, is); }
		void command_remove(istream* is) { console_resolver(COMMAND::remove, is); }
		// void command_copy(istream* is) { console_resolver(COMMAND::copy, is); }
		// void command_move(istream* is) { console_resolver(COMMAND::move, is); }
		void command_set(istream* is) { console_resolver(COMMAND::set, is); }
		void command_unset(istream* is) { console_resolver(COMMAND::unset, is); }
		void command_print(istream* is) { console_resolver(COMMAND::print, is); }
		void command_import(istream* is) { console_resolver(COMMAND::fimport, is); }
		void command_export(istream* is) { console_resolver(COMMAND::fexport, is); }
		void command_merge(istream* is) { console_resolver(COMMAND::merge, is); }
		void command_parse(istream* is) { console_resolver(COMMAND::parse, is); }
		void command_make(istream* is) { console_resolver(COMMAND::make, is); }
		void command_convert(istream* is) { console_resolver(COMMAND::convert, is); }
		void command_tool(istream* is) { console_resolver(COMMAND::tool, is); }
		void command_macro(istream* is) { console_resolver(COMMAND::macro, is); }
		void command_debug(istream* is) { console_resolver(COMMAND::debug, is); }
		void command_preferences(istream* is) { console_resolver(COMMAND::preferences, is); }

		void console_resolver(COMMAND command, istream* is);
		void console_usage(COMMAND hint, bool specs = true);
		void console_print(int opt);
		void console_debug();
		void console_preferences(string type, string val);
		void console_preferences(OBJIO format);
		void console_preferences(HISTORY type);

		void console_file_read(string path);
		void console_file_write(string path);
		void console_e2db_import(ENTRY entry_type, vector<string> paths, int ver = -1, bool dir = false);
		void console_e2db_export(ENTRY entry_type, vector<string> paths, int ver = -1, bool dir = false, string bname = "");
		void console_e2db_merge(ENTRY entry_type, string path, int ver = -1, bool dir = false);
		void console_e2db_merge(ENTRY entry_type, int ver = -1, string bname0 = "", string bname1 = "");
		void console_e2db_parse(ENTRY entry_type, string path, int ver = -1, bool dir = false);
		void console_e2db_make(ENTRY entry_type, string path, int ver = -1, bool dir = false, string bname = "");
		void console_e2db_convert(ENTRY entry_type, int fopt, int ftype, string path, string bname = "", int stype = -1, int ytype = -1);
		void console_e2db_tool(string fn, string bname = "", string prop = "", int order = 0);
		void console_e2db_macro(string id);
		void console_e2db_macro(vector<string> pattern);

		void entry_list(ENTRY entry_type, string bname, int offset0, int offset1);
		void entry_list(ENTRY entry_type, int offset0, int offset1, string bname = "");
		void entry_list(ENTRY entry_type, bool paged = true, int limit = 0, int pos = 0, string bname = "");
		void entry_list(ENTRY entry_type, int pos, int offset, int& end, string bname = "");
		void entry_add(ENTRY entry_type);
		void entry_add(ENTRY entry_type, int ref, string bname);
		void entry_edit(ENTRY entry_type, string id);
		void entry_edit(ENTRY entry_type, int ref, string bname, string id);
		void entry_edit(ENTRY entry_type, bool edit, string id = "", int ref = 0, string bname = "");
		void entry_remove(ENTRY entry_type, int ref, string bname, string id);
		void entry_remove(ENTRY entry_type, string id, string bname = "");
		void entry_parentallock(ENTRY entry_type, string id, bool flag);

		void print_obj_begin(int depth = 0);
		void print_obj_end(int depth = 0);
		void print_obj_sep(int xpos = 0);
		void print_obj_dlm(int depth = 0, int xpos = 0);
		void print_obj_pair(TYPE type, std::any val);
		string obj_escape(ESCAPE esc, VALUE value_type);
		std::any field(TYPE type, bool required = false);

		virtual termiface* term() { return nullptr; }
		static void term_reset() {}
		static int term_paged(int pos, int offset) { return 0; }
		static std::pair<int, int> term_screensize() { return {}; }

		using MSG = e2se::logger::MSG;
		static string msg(string str, string param) { return e2se::logger::msg(str, param); }
		static string msg(string str) { return e2se::logger::msg(str); }
		static string msg(e2se::logger::MSG msg, const char* param) { return e2se::logger::msg(msg, param); }
		static string msg(e2se::logger::MSG msg) { return e2se::logger::msg(msg); }

		std::ostream pout = std::ostream(nullptr);
		std::ostream perr = std::ostream(nullptr);

		e2db* dbih = nullptr;
		e2se::logger* log;
		string last_label;
};

}
#endif /* e2db_console_h */
