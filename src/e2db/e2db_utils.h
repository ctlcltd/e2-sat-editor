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
		void remove_parentallock_lists();
		void remove_bouquets();
		void remove_userbouquets();
		void remove_duplicates();
		void remove_duplicates_transponders();
		void remove_duplicates_services();
		void remove_duplicates_references();
		void remove_duplicates_markers();
		void transform_tunersets_to_transponders();
		void transform_transponders_to_tunersets();
		void sort_transponders();
		void sort_services();
		void sort_userbouquets();
		void sort_references();

	protected:
		virtual e2db_utils* newptr() { return new e2db_utils; }
};
}
#endif /* e2db_utils_h */
