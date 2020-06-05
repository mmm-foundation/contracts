

using namespace eosio;

/**
 * @brief      Структура хоста
 */

struct hosts_struct {
    /**
     * @brief      Returns a string representation of an asset.
     *
     * @param[in]  val   The value
     *
     * @return     String representation of the asset.
     */
    std::string asset_to_string(eosio::asset val){
        uint64_t v = val.symbol.value;
        v >>= 8;
        string result;
        while (v > 0) {
                char c = static_cast<char>(v & 0xFF);
                result += c;
                v >>= 8;
        }
        return result;
    };

    /**
     * @brief      Sets the architect.
     * Устанавливает архитектора сообщества, обладающего полномочиями специальных действий. 
     * 
     * @param[in]  op    The new value
     */
    void set_architect_action (const setarch &op){
        require_auth(op.host);

        account_index accounts(_self, op.host);
        auto acc = accounts.find(op.host);
        eosio_assert(acc != accounts.end(), "Host is not found");
        accounts.modify(acc, _self, [&](auto &a){
            a.architect = op.architect;
        });

    };

    // /**
    //  * @brief      Метод деактивации протокола. 
    //  * Может быть использован для отмены запуска протокола до его фактического запуска в целях изменения параметров. 
    //  * 
    //  * @param[in]  op    The operation
    //  */
    // void deactivate_action(const deactivate &op){
    //     require_auth(op.architect);

    //     account_index accounts(_self, _self);
    //     auto acc = accounts.find(op.host);

    //     eosio_assert(acc != accounts.end(), "Host is not found");
    //     eosio_assert(acc->activated == true, "Host is already deactivated");

    //     eosio_assert(acc->current_pool_num == 1, "Cannot deactivate protocol after rotation launch.");
    //     eosio_assert(acc->current_cycle_num == 1, "Cannot deactivate protocol after rotation launch.");
        
    //     accounts.modify(acc, _self, [&](auto &a){
    //         a.activated = false;
    //     });
    // };



    /**
     * @brief      Метод апгрейда аккаунта до статуса сообщества
     * Принимает ряд параметров, такие как процент консенсуса, реферальный процент, уровни вознаграждений финансовых партнеров, 
     * корпоративный процент, а так же параметры рынка силы. 
     *  
     * @param[in]  op    The operation
     */
    void upgrade_action(const upgrade &op){
        require_auth(op.username);
        
        // eosio_assert(op.purpose.length() > 0, "Purpose should contain a symbols. Describe the idea shortly.");

        //check title lenght
        // eosio_assert((op.title.length() < 1024) && (op.title.length() > 0) , "Title should be more then 10 symbols and less then 1024");
        user_index users(_self,_self);
        auto user = users.find(op.username);
        eosio_assert(user != users.end(), "User is not registered");


        eosio_assert(op.title.length() < 100, "Title is a maximum 100 symbols");
        eosio_assert(op.purpose.find("&#") , "Symbols '&#' is prohibited in purpose");

        eosio_assert(op.quote_amount.amount > 0, "Quote amount must be greater then zero");
        // eosio_assert(op.quote_amount.symbol == _SYM, "Quote symbol for market is only CORE");
        
        eosio_assert(op.consensus_percent <= 100 * PERCENT_PRECISION, "consensus_percent should be between 0 and 100 * PERCENT_PRECISION (1000000)");
        
        eosio_assert(op.referral_percent + op.dacs_percent + op.cfund_percent + op.hfund_percent == 100 * PERCENT_PRECISION, "All payment percents should equal 100 * PERCENT_PRECISION (1000000)");
        
        eosio_assert(op.emission_percent <= 1000 * PERCENT_PRECISION, "emission_percent should be between 0 and 1000 * PERCENT_PRECISION (10000000)");
        eosio_assert(op.gtop <= 100, "Goal top should be less then 100");

        //CHECK for referal levels;
        uint64_t level_count = 0;
        uint64_t percent_count = 0;
        uint64_t prev_level = 100 * PERCENT_PRECISION;

        for (auto level : op.levels){
            level_count++;
            // eosio_assert(level <= prev_level, "Percentage on each referal level should decrease or not change");
            //TODO WHY?
            eosio_assert(level != 0, "Zero-level is prohibited.");
            percent_count += level;
        };

        eosio_assert(percent_count == 100 * PERCENT_PRECISION, "Summ of all levels should be 100%");
        eosio_assert(level_count <= _MAX_LEVELS, "Exceed the maximum number of levels");
        //END CHECK

        account_index accounts(_self, op.username);
        
        auto itr = accounts.find(op.username);
        eosio_assert(itr == accounts.end(), "Account is already upgraded to Host");

        //check and set shares quantity
       
       	eosio_assert(op.total_shares >= 100, "Total shares must be greater or equal 100");
        //check for exist quote and root tokens


        auto failure_if_root_not_exist1 = eosio::token(op.root_token_contract).get_supply(eosio::symbol_type(op.root_token.symbol).name()).amount;
        auto failure_if_root_not_exist2 = eosio::token(op.quote_token_contract).get_supply(eosio::symbol_type(op.quote_amount.symbol).name()).amount;
        
        auto to_pay = op.quote_amount;

        user_index refs(_self, _self);
        auto ref = refs.find(op.username);
        account_name referer = ref->referer;

        std::vector<account_name> empty_dacs;
        std::vector<account_name> empty_fhosts;

        accounts.emplace(_self, [&](auto &a){
            a.username = op.username;
            a.architect = op.username;
            a.hoperator = op.username;
            a.activated = false;
            a.dacs = empty_dacs;
            a.dac_mode = 0;
            a.total_shares = op.total_shares;
            a.quote_amount = op.quote_amount;
            a.quote_symbol = asset_to_string(op.quote_amount);
            a.quote_token_contract = op.quote_token_contract;
            a.quote_precision = op.quote_amount.symbol.precision();
            a.root_token = op.root_token;
            a.voting_only_up = op.voting_only_up;
            a.symbol = asset_to_string(op.root_token);
            a.precision = op.root_token.symbol.precision();
            a.root_token_contract = op.root_token_contract;
            a.meta = op.meta;
            a.registered_at = eosio::time_point_sec(now());
            a.payed = false;
            a.to_pay = to_pay;
            a.ahost = op.username;
            a.fhosts = empty_fhosts;
            a.fhosts_mode = 0;
            a.consensus_percent = op.consensus_percent;
            a.referral_percent = op.referral_percent;
            a.dacs_percent = op.dacs_percent;
            a.cfund_percent = op.cfund_percent;
            a.hfund_percent = op.hfund_percent;
            a.sys_percent = SYS_PERCENT;
            a.levels= op.levels;
        });

        ahosts_index ahosts(_self, _self);
        ahosts.emplace(op.username, [&](auto &a){
            a.username = op.username;
            a.title = op.title;
            a.purpose = op.purpose;
            a.manifest = "";
            a.comments_is_enabled = false;
        });


        users.modify(user, _self, [&](auto &u){
            u.is_host = true;
        });

        emission_index emis(_self, op.username);
        auto emi = emis.find(op.username);
        eosio_assert(emi == emis.end(), "Emission object already created");

        emis.emplace(op.username, [&](auto &e){
            e.host = op.username;
            e.gtop = op.gtop;
            e.percent = op.emission_percent;
            e.fund = asset(0, op.root_token.symbol);
        });


    }

    /**
     * @brief      Метод создания дочернего хоста
     * Позволяет сообществу на границе циклов изменять параметры финансового ядра, 
     * распологая их в области памяти аккаунта дочернего хоста. 
     *
     * @param[in]  op    The operation
     */
    void create_chost_action(const cchildhost &op){
    	auto parent_host = op.parent_host;
    	auto chost = op.chost;
    	account_index hosts(_self, parent_host);
    	auto acc = hosts.find(parent_host);
    	eosio_assert(acc != hosts.end(), "Parent host is not exist");
    	require_auth(parent_host);
    	eosio_assert(acc->activated == true, "Main host should be activated before set a child host");
    	eosio_assert( is_account( chost ), "Child account does not exist");
    	
    	std::vector<account_name> childs = acc->chosts;

    	//check for exist in main hosts;
    	auto is_exist = hosts.find(chost);
    	eosio_assert(is_exist == hosts.end(), "Child host already registered like father host");
		eosio_assert(acc->non_active_chost == false, "Founded not activated child host");
    	
    	// //Check for exist not active child
    	if (childs.begin() != childs.end()){
    		account_name last_child = childs.back() - 1;
		}
    	
    	childs.push_back(chost);

        auto root_symbol = acc->get_root_symbol();
        
    	eosio::asset to_pay = asset(10000, root_symbol);
        
    	hosts.modify(acc, parent_host, [&](auto &h){
    		h.chosts = childs;
    		h.payed = false;
    		h.to_pay = to_pay;
    		h.parameters_setted = false;
    		h.non_active_chost = true;
    	});


    }

    /**
     * @brief      Метод оплаты комиссии создания дочернего хоста
     *
     * @param[in]  username  The username
     * @param[in]  amount    The amount
     */
    void pay_for_upgrade(account_name username, eosio::asset amount, account_name code){
    	account_index hosts(_self, username);

    	auto host = hosts.find(username);
    	eosio_assert(host != hosts.end(), "Host is not founded");
        
   		//PAY for upgrade only in CORE!.
    	eosio_assert(amount.symbol == host->quote_amount.symbol, "Wrong symbol for market");
        
        eosio_assert(code == host->quote_token_contract, "Wrong symbol for market");
        
	   	//Check for enough upgrade amount for quote asset
    	eosio_assert(amount == host->to_pay, "Amount is not equal amount for upgrade");
    	eosio_assert(host->payed == 0, "Already payed");

    	std::vector<account_name> childs = host->chosts;
	   
    	hosts.modify(host, _self, [&](auto &h){
			h.payed = true;
			h.to_pay = asset(0, _SYM);
		});    	

        if (childs.begin() == childs.end())
    	{
    		shares().create_bancor_market("POWER MARKET", 0, username, host->total_shares, host->quote_amount, host->quote_token_contract, _SHARES_VESTING_DURATION);
		};

	  
    };

    // /**
    //  * @brief      Метод редактирования времени 
    //  *
    //  * @param[in]  op    The operation
    //  */
    // void ehosttime_action(const ehosttime &op){
    //     require_auth (op.architect);

    //     account_index hosts(_self, _self);
    //     auto host = hosts.find(op.host);
    //     eosio_assert(host->architect == op.architect, "You are not architect of currenty community");
        
    //     eosio_assert((op.priority_seconds < 7884000), "Pool Priority Seconds must be greater or equal then 0 sec and less then 7884000 sec");
    //     eosio_assert((op.pool_timeout >= 60) && (op.pool_timeout < 7884000),"Pool Timeout must be greater or equal then 1 sec and less then 7884000 sec");
    
    //     spiral_index spiral(_self, op.host);
    //     auto sp = spiral.find(0);

    //     spiral.modify(sp, op.architect, [&](auto &s){
    //         s.pool_timeout = op.pool_timeout;
    //         s.priority_seconds = op.priority_seconds;
    //     });
    // }




    /**
     * @brief      Метод редактирования информации о хосте
     *
     * @param[in]  op    The operation
     */
    void edithost_action(const edithost &op){
        require_auth (op.architect);

        account_index hosts(_self, op.host);
        auto host = hosts.find(op.host);
        eosio_assert(host->architect == op.architect, "You are not architect of current community");

        eosio_assert(host != hosts.end(), "Host is not founded");
        
        // eosio_assert(op.purpose.length() > 0, "Purpose should contain a symbols. Describe the idea shortly.");

        //check title lenght
        // eosio_assert((op.title.length() < 1024) && (op.title.length() > 0) , "Title should be more then 10 symbols and less then 1024");
        

        // hosts.modify(host, op.architect, [&](auto &h){
        //     h.purpose = op.purpose;
        //     h.meta = op.meta;
        //     h.title = op.title;
        // });

        ahosts_index ahosts(_self, _self);
        auto ahost = ahosts.find(op.host);

        ahosts.modify(ahost, op.architect, [&](auto &a){
            a.title = op.title;
            a.purpose = op.purpose;
            a.manifest = op.manifest;
        });


        
    }

};