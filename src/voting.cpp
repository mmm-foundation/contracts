using namespace eosio;

	void clear_old_votes_action (eosio::name voter, eosio::name host){
		votes_index votes(_me, voter.value);
		goals_index goals (_me, host.value);

		auto idx = votes.begin();
		std::vector<uint64_t> list_for_delete;

		while (idx !=votes.end()){
			auto goal = goals.find(idx->goal_id);
			if (goal->status == "filled"_n){
				list_for_delete.push_back(idx->goal_id);
			};
			idx++;
		}
		for (auto i : list_for_delete){
			auto vote_for_delete = votes.find(i);
			votes.erase(vote_for_delete);
		}
	}

	uint64_t count_votes(eosio::name voter, eosio::name host) {
		votes_index votes(_me, voter.value);

		goals_index goals (_me, host.value);
		// auto users_with_id = reports.template get_index<"userwithtask"_n>();

		// auto idx = votes.begin();

		auto idx = votes.template get_index<"host"_n>();
		auto i_bv = idx.lower_bound(host.value);

		uint64_t count = 0;

		while (i_bv != idx.end() && (i_bv->host == host)){
			count++;
			i_bv++;
		}
		print("count: ", count);
		return count;
	}


	uint64_t count_rvotes(eosio::name voter, eosio::name host, uint64_t task_id) {
		rvotes_index rvotes(_me, voter.value);

		goals_index goals (_me, host.value);

		// auto users_with_id = reports.template get_index<"userwithtask"_n>();
		// auto idx = votes.begin();

		auto idx = rvotes.template get_index<"bytaskid"_n>();
		auto i_bv = idx.begin();

		uint64_t count = 0;

		while (i_bv != idx.end() && (i_bv->task_id == task_id)){
			count++;
			i_bv++;
		}
		
		print("count: ", count);

		return count;
	}


//Метод голосования за цель
	[[eosio::action]] void unicore::vote(eosio::name voter, eosio::name host, uint64_t goal_id, bool up){
		eosio::check(has_auth(voter) || has_auth(_me), "missing required authority");
    
    eosio::name payer;

    if (has_auth(voter)) {
        payer = voter;
    } else payer = _me;

		
		partners2_index users(_partners,_partners.value);
    auto user = users.find(voter.value);
    eosio::check(user != users.end(), "User is not registered");

		uint64_t vote_count = count_votes(voter, host);
		
		goals_index goals(_me, host.value);
		power3_index power(_me, host.value);
		votes_index votes(_me, voter.value);

		auto goal = goals.find(goal_id);
		account_index accounts (_me, (goal->host).value);
		auto acc = accounts.find((goal->host).value);

		market_index market(_me, host.value);
		auto itr = market.find(0);
		auto liquid_shares = acc->total_shares - itr->base.balance.amount;

		// print("LIQUIDSHARES ", liquid_shares);

		eosio::check(goal != goals.end(), "Goal is not founded");

		auto pow = power.find(voter.value);
		eosio::check(pow != power.end(), "You dont have shares for voting process");
		eosio::check(pow -> power != 0, "You cant vote with zero power");
		
		auto voters = goal -> voters;
		

		// auto vote = votes.find(goal->id);
		auto goal_idwithhost_idx = votes.template get_index<"idwithhost"_n>();
		auto votes_ids = combine_ids(host.value, goal->id);
		
		auto vote = goal_idwithhost_idx.find(votes_ids);

		// eosio::check(vote!= goal_idwithhost_idx.end(), "this is end");

		if (vote == goal_idwithhost_idx.end()){
			//ADD VOTE
			uint64_t max_votes = unicore::getcondition(host, "maxuvotes");
			
			eosio::check(vote_count <= max_votes, "Votes limit is exceeded");

      // eosio::check(goal->filled == false, "You cant vote for filled goal");
      
      if (up == false){
      	eosio::check(acc->voting_only_up == false, "Downvoting is prohibited");
      }

      voters.push_back(voter);
      
      goals.modify(goal, payer, [&](auto &g){
       	if (goal -> status == "waiting"_n || goal -> status == "validated"_n) {
					up == true ? g.positive_votes = goal->positive_votes + pow -> power : g.negative_votes =  goal->negative_votes + pow->power;
       	}

       	g.voters = voters;

       	int64_t votes_percent = g.positive_votes * 100 * ONE_PERCENT / liquid_shares;
       	if (goal -> status == "waiting"_n) {
       		g.status = (votes_percent >= acc->consensus_percent && g.positive_votes > 0) ? "validated"_n : "waiting"_n;	
       	}
       	
		
      });

      votes.emplace(payer, [&](auto &v){
      	v.id = votes.available_primary_key();
      	v.goal_id = goal->id;
      	v.host = goal->host;
      	v.power = up == true ? pow -> power : -pow->power;
      });
	        
		} else {
			
			auto voters = goal -> voters;
			auto itr = std::find(voters.begin(), voters.end(), voter);
			voters.erase(itr);
			goals.modify(goal, payer, [&](auto &g){
				g.voters = voters;

				if (goal -> status == "waiting"_n || goal -> status == "validated"_n) {
					vote->power < 0 ? g.negative_votes = goal->negative_votes + vote->power : g.positive_votes = goal->positive_votes - vote->power;
				}

				int64_t votes_percent = g.positive_votes * ONE_PERCENT / liquid_shares;
				
				
				if (goal -> status == "waiting"_n) {
       		g.status = (votes_percent >= acc->consensus_percent / 100 && g.positive_votes > 0) ? "validated"_n : "waiting"_n;	
       	}
       	
			});

			goal_idwithhost_idx.erase(vote);
		}
		// clear_old_votes_action(voter, host);
		
		
	}

//Метод голосования за отчёт
	[[eosio::action]] void unicore::rvote(eosio::name voter, eosio::name host, uint64_t report_id, bool up){
		require_auth(voter);
		
		partners2_index users(_partners,_partners.value);
    auto user = users.find(voter.value);
    eosio::check(user != users.end(), "User is not registered");

		account_index accounts (_me, host.value);
		auto acc = accounts.find(host.value);

		goals_index goals(_me, host.value);
		tasks_index tasks(_me, host.value);
		power3_index power(_me, host.value);

		rvotes_index rvotes(_me, voter.value);
		reports_index reports(_me, host.value);

		auto report = reports.find(report_id);
		auto task_id = report -> task_id;
		auto task = tasks.find(task_id);
		auto goal = goals.find(report -> goal_id);

		
		eosio::check(task != tasks.end(), "Task is not found");

		eosio::check(report -> approved == true, "Report is not approved and cant participate on the voting");

		uint64_t vote_count = count_rvotes(voter, host, task_id);

		auto pow = power.find(voter.value);
		eosio::check(pow != power.end(), "You dont have shares for voting process");
		eosio::check(pow -> power != 0, "You cant vote with zero power");
		
		auto voters = report -> voters;

		auto reports_by_task_idx = rvotes.template get_index<"idwithhost"_n>();
		auto report_ids = combine_ids(host.value, task_id);
		
		auto vote = reports_by_task_idx.find(report_ids);
		auto _VOTES = eosio::symbol(eosio::symbol_code("VOTES"), 0);
      

		if (vote == reports_by_task_idx.end()){
			//ADD VOTE
			eosio::check(goal->finish_at.sec_since_epoch() > eosio::current_time_point().sec_since_epoch(), "Voting is completed");

			eosio::check(vote_count < _TOTAL_VOTES, "Votes limit is exceeded");

      // eosio::check(goal->filled == false, "You cant vote for filled goal");
      
      if (up == false){
      	eosio::check(acc->voting_only_up == false, "Downvoting is prohibited");
      }

      voters.push_back(voter);
      
      reports.modify(report, voter, [&](auto &r) {
				up == true ? r.positive_votes = report->positive_votes + pow -> power : r.negative_votes =  report->negative_votes + pow->power;
       	r.voters = voters;
			});   

      tasks.modify(task, voter, [&](auto &g){
       	up == true ? g.positive_votes = task->positive_votes + pow -> power : g.negative_votes =  task->negative_votes + pow->power;
      });

			
      rvotes.emplace(voter, [&](auto &v){
      	v.id = rvotes.available_primary_key();
      	v.task_id = task_id;
      	v.host = host;
      	v.power = up == true ? pow -> power : -pow->power;
      });


      goals.modify(goal, voter, [&](auto &g) {
      	g.target3 = asset(goal->target3.amount + pow -> power, _VOTES);
      });

		} else {

			auto voters = report -> voters;
			auto itr = std::find(voters.begin(), voters.end(), voter);

			voters.erase(itr);

			reports.modify(report, voter, [&](auto &r) {
       	vote -> power < 0 ? r.negative_votes = report->negative_votes + vote->power : r.positive_votes = report->positive_votes - vote->power;
       	r.voters = voters;
			}); 

			tasks.modify(task, voter, [&](auto &g) {
				vote -> power < 0 ? g.negative_votes = task->negative_votes + vote->power : g.positive_votes = task->positive_votes - vote->power;
			});

			goals.modify(goal, voter, [&](auto &g) {
				vote -> power < 0 ? g.target3 = asset(goal->target3.amount + abs(vote->power), _VOTES) : g.target3 = asset(goal->target3.amount - vote->power, _VOTES);
      });

			reports_by_task_idx.erase(vote);

		}

		// clear_old_votes_action(voter, host);
		
		
	}

	