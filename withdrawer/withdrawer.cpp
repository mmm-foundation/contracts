
#include "withdrawer.hpp"

/**
\defgroup public_consts CONSTS
\brief Константы контракта
*/

/**
\defgroup public_actions ACTIONS
\brief Методы действий контракта
*/

/**
\defgroup public_tables TABLES
\brief Структуры таблиц контракта
*/


void withdrawer::complete(name host, name admin, uint64_t withdraw_id, std::string trx_id)
{
  require_auth( admin );
  admins_index admins(_me, host.value);
  auto adm = admins.find(admin.value);
  eosio::check(adm != admins.end(), "Admin is not found");

  withdraws_index withdraws(_me, host.value); 
  auto withdraw = withdraws.find(withdraw_id);
  eosio::check(withdraw != withdraws.end(), "Object is not found");
  
  eosio::check(withdraw -> status == "process"_n, "Only processed transactions can be completed");

  withdraws.modify(withdraw, _me, [&](auto &w){
    w.status = "completed"_n;
    w.trx_id = trx_id;
  });

  action(
      permission_level{ _me, "active"_n },
      withdraw -> contract, "retire"_n,
      std::make_tuple( _me, withdraw -> amount, trx_id) 
  ).send();

}




void withdrawer::cancel(name host, name admin, uint64_t withdraw_id, std::string cancel_reason)
{
  require_auth( admin );
  admins_index admins(_me, host.value);
  auto adm = admins.find(admin.value);
  eosio::check(adm != admins.end(), "Admin is not found");

  withdraws_index withdraws(_me, host.value); 
  auto withdraw = withdraws.find(withdraw_id);
  eosio::check(withdraw != withdraws.end(), "Object is not found");

  eosio::check(withdraw -> status == "process"_n, "Only processed transactions can be cancelled");


  withdraws.modify(withdraw, _me, [&](auto &w){
    w.status = "cancelled"_n;
    w.cancel_reason = cancel_reason;
  });

  action(
      permission_level{ _me, "active"_n },
      withdraw->contract, "transfer"_n,
      std::make_tuple( _me, withdraw -> username, withdraw -> amount, cancel_reason) 
  ).send();

}



void withdrawer::add_withdraw(name contract, name username, asset amount, name host, std::string address){
  withdraws_index withdraws(_me, host.value);
  
  withdraws.emplace(_me, [&](auto &w){
    w.id = withdraws.available_primary_key();
    w.username = username;
    w.contract = contract;
    w.host = host;
    w.status = "process"_n;
    w.amount = amount;
    w.address = address;
  });
}



void withdrawer::addadmin(name host, name admin)
{
  require_auth( host );
    
  admins_index admins(_me, host.value); 
  auto adm = admins.find(admin.value);
  eosio::check(adm == admins.end(), "Admin is already exist");

  admins.emplace(_me, [&](auto &a){
    a.username = admin;
  });
  
}


void withdrawer::rmadmin(name host, name admin)
{
  require_auth( host );
    
  admins_index admins(_me, host.value); 
  auto adm = admins.find(admin.value);
  eosio::check(adm != admins.end(), "Admin is not found");

  admins.erase(adm);
  
}


extern "C" {
   void apply( uint64_t receiver, uint64_t code, uint64_t action ) {
        if (code == withdrawer::_me.value) {
          
          if (action == "complete"_n.value) {
            execute_action(name(receiver), name(code), &withdrawer::complete);
          } else if (action == "cancel"_n.value){
            execute_action(name(receiver), name(code), &withdrawer::cancel);
          } else if (action == "addadmin"_n.value){
            execute_action(name(receiver), name(code), &withdrawer::addadmin);
          } else if (action == "rmadmin"_n.value){
            execute_action(name(receiver), name(code), &withdrawer::rmadmin);
          }

        } else {

          if (action == "transfer"_n.value) {
            
            struct transfer {
                eosio::name from;
                eosio::name to;
                eosio::asset quantity;
                std::string memo;
            };

            auto op = eosio::unpack_action_data<transfer>();

            if (op.to == withdrawer::_me) {

                eosio::name host; 
                std::string message = "";

                auto delimeter = op.memo.find('-');
        
                auto host_string = op.memo.substr(0, delimeter);
                
                host = name(host_string.c_str());
                
                auto memo = op.memo.substr(delimeter + 1, op.memo.length());
              
                
                withdrawer::add_withdraw(name(code), op.from, op.quantity, host, memo);
                
            }
          }
        }
  };
};
