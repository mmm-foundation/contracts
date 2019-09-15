
namespace eosio {
    uint128_t combine_ids(const uint64_t &x, const uint64_t &y) {
        return (uint128_t{x} << 64) | y;
    }


	//@abi table tasks
	struct tasks{
		uint64_t task_id;
		uint64_t goal_id;
		uint64_t priority;
		uint64_t period;
		bool is_public = true;
		eosio::string title;
		eosio::string data;
		eosio::asset requested;
		eosio::asset funded;
		eosio::asset remain;
		eosio::asset for_each;
		account_name curator;
		bool with_badge;
		uint64_t badge_type;
		bool validated = true;
		bool completed = false;
		bool active = true;
		eosio::time_point_sec expired_at;

		uint64_t primary_key()const { return task_id; }

	    EOSLIB_SERIALIZE( tasks, (task_id)(goal_id)(priority)(period)(is_public)(title)(data)(requested)(funded)(remain)(for_each)(curator)(with_badge)(badge_type)(validated)(completed)(active)(expired_at))
    };

    typedef eosio::multi_index< N(tasks), tasks> tasks_index;


    //@abi table reports
    struct reports{
    	uint64_t report_id;
    	uint64_t task_id; 
    	uint64_t goal_id;
    	account_name username;
    	account_name curator;
    	eosio::string data;
    	eosio::asset requested;
    	bool need_check = true;
    	bool approved = false;
    	eosio::string comment;

    	uint64_t primary_key() const {return report_id;}
		uint128_t user_with_task() const { return combine_ids(username, task_id); }

    	EOSLIB_SERIALIZE(reports, (report_id)(task_id)(goal_id)(username)(curator)(data)(requested)(need_check)(approved)(comment))
    };

    typedef eosio::multi_index< N(reports), reports,
    	indexed_by<N(user_with_task), const_mem_fun<reports, uint128_t, 
                              &reports::user_with_task>>
    > reports_index;


	//@abi action
	struct settask {
		account_name host;
		uint64_t goal_id;
		eosio::string title;
		eosio::string data;
		eosio::asset requested;
		bool is_public;
		eosio::asset for_each;
		bool with_badge;
		uint64_t badge_type;
		uint64_t expiration;
	};

	//@abi action
	struct fundtask {
		account_name host;
		uint64_t task_id;
		eosio::asset amount;
		eosio::string comment;
	};

	//@abi action
	struct tactivate
	{
		account_name host;
		uint64_t task_id;
	};
	
	//@abi action
	struct tdeactivate
	{
		account_name host;
		uint64_t task_id;
	};

	//@abi action
	struct setreport {
		account_name host;
		account_name username;
		uint64_t task_id;
		eosio::string data;
	};

	//@abi action
	struct editreport{
		account_name host;
		account_name username;
		uint64_t report_id;
		eosio::string data;
	};

	//@abi action
	struct approver{
		account_name host;
		uint64_t report_id;
		eosio::string comment;

	};

	//@abi action
	struct disapprover{
		account_name host;
		uint64_t report_id;
		eosio::string comment;
	};
}