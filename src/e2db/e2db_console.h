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
#include <map>
#include <any>

using std::string, std::vector;

#ifndef e2db_console_h
#define e2db_console_h
#include "../logger/logger.h"
#include "../e2db/e2db.h"

namespace e2se_e2db
{

struct streamiface
{
	public:
		virtual ~streamiface() = default;

		virtual streamiface &operator<<(int i) = 0;
		virtual streamiface &operator<<(char c) = 0;
		virtual streamiface &operator<<(const char* s) = 0;
		virtual streamiface &operator<<(const std::string &s) = 0;
		virtual streamiface &operator<<(const streamiface &) = 0;

		virtual streamiface &endl() = 0;
		virtual streamiface &flush() = 0;
		virtual streamiface &left() = 0;
		virtual streamiface &right() = 0;
		virtual int width() const = 0;
		virtual int width(int width) = 0;
};

struct termiface
{
	public:

		enum HANDLE {
			Command,
			Listing,
			Input
		};

		virtual ~termiface() = default;
		virtual void handler(HANDLE handle) = 0;
		virtual void clear() = 0;
		virtual std::istream* ptr() = 0;
		virtual const std::string line() = 0;
		virtual const std::string token() = 0;
		virtual void reset() = 0;
		virtual int paged(int pos, int offset) = 0;
		virtual std::pair<int, int> screensize() = 0;
		virtual void dump_log() = 0;
		virtual void load_history() = 0;
		virtual void save_history() = 0;
};

class e2db_console
{
	public:
		virtual ~e2db_console() = default;
		std::string editor_version();
		int exited();

	protected:

		static const int PAGED_LIMIT = 10;

		enum COMMAND {
			usage,
			version,
			quit,
			add,
			edit,
			remove,
			list,
			fread,
			fwrite,
			fimport,
			fexport,
			copy,
			move,
			set,
			unset,
			merge,
			parse,
			make,
			convert,
			tool,
			macro,
			print,
			inspect,
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

		virtual void console_exit();
		virtual void console_header();
		virtual void console_error(const string& cmd);
		virtual void console_version(bool extended = false);
		virtual void console_resolver(COMMAND command, istream* is);
		virtual void console_usage(COMMAND hint, int level = 3);
		virtual void console_print(int opt);
		virtual void console_inspect();
		virtual void console_preferences(string type, string val);
		virtual void console_preferences(OBJIO format);
		virtual void console_preferences(HISTORY type);

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
		void command_inspect(istream* is) { console_resolver(COMMAND::inspect, is); }
		void command_preferences(istream* is) { console_resolver(COMMAND::preferences, is); }
		void command_version() { console_version(); }
		void command_quit() { console_exit(); }

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

		virtual void entry_list(ENTRY entry_type, string bname, int offset0, int offset1);
		virtual void entry_list(ENTRY entry_type, int offset0, int offset1, string bname = "");
		virtual void entry_list(ENTRY entry_type, bool paged = true, int limit = 0, int pos = 0, string bname = "");
		virtual void entry_add(ENTRY entry_type);
		virtual void entry_add(ENTRY entry_type, int ref, string bname);
		virtual void entry_edit(ENTRY entry_type, string id);
		virtual void entry_edit(ENTRY entry_type, int ref, string bname, string id);
		virtual void entry_edit(ENTRY entry_type, bool edit, string id = "", int ref = 0, string bname = "");
		virtual void entry_remove(ENTRY entry_type, int ref, string bname, string id);
		virtual void entry_remove(ENTRY entry_type, string id, string bname = "");
		virtual void entry_parentallock(ENTRY entry_type, string id, bool flag);
		virtual void entry_list_exec(ENTRY entry_type, int pos, int offset, int& end, string bname = "");
		virtual void entry_edit_exec(ENTRY entry_type, bool edit, string id = "", int ref = 0, string bname = "");
		virtual void entry_remove_exec(ENTRY entry_type, string id, string bname = "");
		virtual void entry_parentallock_exec(ENTRY entry_type, string id, bool flag);

		bool label_field(TYPE type, string &label, string &description);
		void label_obj_pair(TYPE type, string &name, VALUE &value_type);
		bool value_field(TYPE type, string str, bool required, std::any &val);
		void value_obj_pair(TYPE type, VALUE value_type, std::any val, int &d, string &str);
		virtual std::any field(TYPE type, bool required = false);
		virtual map<int, vector<pair<TYPE, bool>>> input_mask(ENTRY entry_type, bool edit, string id = "", int ref = 0, string bname = "");

		string obj_escape(ESCAPE esc, VALUE value_type);
		void print_obj_begin(int depth = 0);
		void print_obj_end(int depth = 0);
		void print_obj_sep(int xpos = 0);
		void print_obj_dlm(int depth = 0, int xpos = 0);
		void print_obj_pair(TYPE type, std::any val);

		using MSG = e2se::logger::MSG;
		static string msg(string str, string param) { return e2se::logger::msg(str, param); }
		static string msg(string str) { return e2se::logger::msg(str); }
		static string msg(e2se::logger::MSG msg, const char* param) { return e2se::logger::msg(msg, param); }
		static string msg(e2se::logger::MSG msg) { return e2se::logger::msg(msg); }

		termiface* termctl = nullptr;
		streamiface* pout = nullptr;
		streamiface* perr = nullptr;

		e2db* dbih = nullptr;
		e2se::logger* plog;
		string curr_field;
};

}
#endif /* e2db_console_h */
