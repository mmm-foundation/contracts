#pragma once

#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>

#include <string>

namespace eosiosystem {
   class system_contract;
}

namespace eosio {

   using std::string;

   class [[eosio::contract("eosio.token")]] token : public contract {
      public:
         using contract::contract;

         static uint128_t combine_ids(const uint64_t &x, const uint64_t &y) {
            return (uint128_t{x} << 64) | y;
         };

         [[eosio::action]]
         void create( name   issuer,
                      asset  maximum_supply);

         [[eosio::action]]
         void issue( name to, asset quantity, string memo );

         [[eosio::action]]
         void retire( name username, asset quantity, string memo );

         [[eosio::action]]
         void transfer( name    from,
                        name    to,
                        asset   quantity,
                        string  memo );

         [[eosio::action]]
         void open( name owner, const symbol& symbol, name ram_payer );

         [[eosio::action]]
         void close( name owner, const symbol& symbol );

         static asset get_supply( name token_contract_account, symbol_code sym_code )
         {
            stats statstable( token_contract_account, sym_code.raw() );
            const auto& st = statstable.get( sym_code.raw() );
            return st.supply;
         }

         static asset get_balance( name token_contract_account, name owner, symbol_code sym_code )
         {
            accounts accountstable( token_contract_account, owner.value );
            const auto& ac = accountstable.get( sym_code.raw() );
            return ac.balance;
         }

         using create_action = eosio::action_wrapper<"create"_n, &token::create>;
         using issue_action = eosio::action_wrapper<"issue"_n, &token::issue>;
         using retire_action = eosio::action_wrapper<"retire"_n, &token::retire>;
         using transfer_action = eosio::action_wrapper<"transfer"_n, &token::transfer>;
         using open_action = eosio::action_wrapper<"open"_n, &token::open>;
         using close_action = eosio::action_wrapper<"close"_n, &token::close>;
      private:
         struct [[eosio::table]] account {
            asset    balance;

            uint64_t primary_key()const { return balance.symbol.code().raw(); }
         };

         struct [[eosio::table]] globalbal {
            uint64_t id;
            name username;
            asset    balance;

            uint64_t primary_key()const { return id; }
            
            uint128_t userandsym() const { return combine_ids(username.value, balance.symbol.code().raw()); }
            uint128_t symandamount() const { return combine_ids(balance.symbol.code().raw(), - balance.amount);}
        
            // uint64_t primary_key()const { return balance.symbol.code().raw(); }
         };


         struct [[eosio::table]] currency_stats {
            asset    supply;
            asset    max_supply;
            name     issuer;

            uint64_t primary_key()const { return supply.symbol.code().raw(); }
         };

         typedef eosio::multi_index< "accounts"_n, account > accounts;


         typedef eosio::multi_index<"globalbal"_n, globalbal,
           eosio::indexed_by<"userandsym"_n, eosio::const_mem_fun<globalbal, uint128_t, &globalbal::userandsym>>,
           eosio::indexed_by<"symandamount"_n, eosio::const_mem_fun<globalbal, uint128_t, &globalbal::symandamount>>
          > globalbal_index;
          

         typedef eosio::multi_index< "stat"_n, currency_stats > stats;


         void sub_balance( name owner, asset value );
         void add_balance( name owner, asset value, name ram_payer );
   };

} /// namespace eosio
