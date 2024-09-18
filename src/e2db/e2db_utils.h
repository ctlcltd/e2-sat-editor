/*!
 * e2-sat-editor/src/e2db/e2db_utils.h
 *
 * @link https://github.com/ctlcltd/e2-sat-editor
 * @copyright e2 SAT Editor Team
 * @author Leonardo Laureti
 * @version 1.6.0
 * @license MIT License
 * @license GNU GPLv3 License
 */

#include "../logger/logger.h"
#include "e2db_abstract.h"

#ifndef e2db_utils_h
#define e2db_utils_h
namespace e2se_e2db
{
class e2db;

//TODO
class e2db_utils : virtual public e2db_abstract
{
	public:

		enum SORT_ITEM {
			item_transponder,
			item_service,
			item_userbouquet,
			item_reference
		};

		enum SORT_ORDER {
			sort_asc,
			sort_desc
		};

		struct sort_data
		{
			enum TYPE {
				integer,
				string,
				boolean
			};

			struct value
			{
				value(void* ptr, sort_data::TYPE type)
				{
					this->type = type;
					this->ptr = ptr;
				}
				sort_data::TYPE type;
				void* ptr;
				int val_integer() { return (*(int*) ptr); }
				bool val_boolean() { return (*(bool*) ptr); }
				std::string val_string() { return (*(std::string*) ptr); }
			};

			vector<sort_data::value*> data;
			void insert(void* ptr, sort_data::TYPE type)
			{
				data.emplace_back(new sort_data::value(ptr, type));
			}
			void push(int* ptr) { insert(ptr, sort_data::integer); }
			void push(bool* ptr) { insert(ptr, sort_data::boolean); }
			void push(std::string* ptr) { insert(ptr, sort_data::string); }
		};

		e2db_utils();
		virtual ~e2db_utils() = default;

		void remove_orphaned_services();
		void remove_orphaned_references();
		void fix_remove_references();
		void fix_dvbns();
		void clear_services_cached();
		void clear_services_caid();
		void clear_services_flags();
		void clear_services_data();
		void clear_favourites();
		void clear_bouquets_unused_services();
		void remove_parentallock();
		void remove_parentallock_services();
		void remove_parentallock_userbouquets();
		void remove_bouquets();
		void remove_userbouquets();
		void remove_duplicates();
		void remove_duplicates_transponders();
		void remove_duplicates_services();
		void remove_duplicates_references();
		void remove_duplicates_markers();
		void transform_tunersets_to_transponders();
		void transform_transponders_to_tunersets();
		void sort_transponders(string prop = "tsid", SORT_ORDER order = SORT_ORDER::sort_asc);
		void sort_transponders(vector<pair<int, string>> xis, int start, int end, string prop = "tsid", SORT_ORDER order = SORT_ORDER::sort_asc);
		void sort_services(string prop = "ssid", SORT_ORDER order = SORT_ORDER::sort_asc);
		void sort_services(vector<pair<int, string>> xis, int start, int end, string prop = "ssid", SORT_ORDER order = SORT_ORDER::sort_asc);
		void sort_userbouquets(string prop = "bname", SORT_ORDER order = SORT_ORDER::sort_asc);
		void sort_userbouquets(vector<pair<int, string>> xis, int start, int end, string prop = "bname", SORT_ORDER order = SORT_ORDER::sort_asc);
		void sort_references(bool c, string prop = "chname", SORT_ORDER order = SORT_ORDER::sort_asc);
		void sort_references(string bname, string prop = "chname", SORT_ORDER order = SORT_ORDER::sort_asc);
		void sort_references(string bname, vector<pair<int, string>> xis, int start, int end, string prop = "chname", SORT_ORDER order = SORT_ORDER::sort_asc);

	protected:
		virtual e2db_utils* newptr() { return new e2db_utils; }
		void rebuild_index_transponders();
		void rebuild_index_services();
		void rebuild_index_services(unordered_set<string> i_services); // insert
		void rebuild_index_userbouquet(string iname);
		void rebuild_index_userbouquet(string iname, unordered_set<string> i_channels); // remove
		void rebuild_index_markers();
		sort_data get_data(SORT_ITEM model, string prop, string iname, vector<pair<int, string>>);
		void sort_items(SORT_ITEM model, string prop, SORT_ORDER order, string iname, vector<pair<int, string>> xis, int start = 0, int end = 0);
		static string value_sort_order(SORT_ORDER order);

	private:
		static bool valueLessThan(const pair<sort_data::value*, int>& left, const pair<sort_data::value*, int>& right)
		{
			switch (left.first->type)
			{
				case sort_data::integer: return left.first->val_integer() < right.first->val_integer();
				case sort_data::string: return left.first->val_string() < right.first->val_string();
				case sort_data::boolean: return left.first->val_boolean() < right.first->val_boolean();
				default: return false;
			}
		}
		static bool valueGreaterThan(const pair<sort_data::value*, int>& left, const pair<sort_data::value*, int>& right)
		{
			switch (right.first->type)
			{
				case sort_data::integer: return right.first->val_integer() < left.first->val_integer();
				case sort_data::string: return right.first->val_string() < left.first->val_string();
				case sort_data::boolean: return right.first->val_boolean() < left.first->val_boolean();
				default: return false;
			}
		}
};
}
#endif /* e2db_utils_h */
