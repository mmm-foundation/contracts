#include "reserve.hpp"
#include <eosio/transaction.hpp>

using namespace eosio;


void reserve::reservation(eosio::name contract, eosio::name username, eosio::asset quantity, std::string memo) {
    action(
        permission_level{ _me, "active"_n },
        "eosio"_n, "putreserve"_n,
        std::make_tuple(contract, username, quantity, memo)
    ).send();
}

[[eosio::action]] void reserve::update() {

}


extern "C" {
   
   /// The apply method implements the dispatch of events to this contract
   void apply( uint64_t receiver, uint64_t code, uint64_t action ) {
        if (code == _me.value) {
          if (action == "update"_n.value){
            execute_action(name(receiver), name(code), &reserve::update);
          }
        } else {
          if (action == "transfer"_n.value){
            
            struct transfer{
                eosio::name from;
                eosio::name to;
                eosio::asset quantity;
                std::string memo;
            };

            auto op = eosio::unpack_action_data<transfer>();

            if (op.to == _me){
              reserve::reservation(name(code), op.from, op.quantity, op.memo);
            }
          }
        }
  };
};
