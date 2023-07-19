#include <algorithm>
#include <cmath>
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/time.hpp>
#include <eosio/multi_index.hpp>
#include <eosio/contract.hpp>
#include <eosio/action.hpp>
#include <eosio/system.hpp>
#include <eosio/print.hpp>
#include <eosio/datastream.hpp>

using namespace eosio;



class [[eosio::contract]] withdrawer : public eosio::contract {

public:
    withdrawer( eosio::name receiver, eosio::name code, eosio::datastream<const char*> ds ): eosio::contract(receiver, code, ds)
    {}
    void apply(uint64_t receiver, uint64_t code, uint64_t action);   
    
    static void add_withdraw(name contract, name username, asset amount, name host, std::string address);
    
    [[eosio::action]]
    void complete(name host, name admin, uint64_t withdraw_id, std::string trx_id);

    [[eosio::action]]
    void cancel(name host, name admin, uint64_t withdraw_id, std::string cancel_reason);
   
    [[eosio::action]]
    void addadmin(name host, name admin);
   
   [[eosio::action]]
    void rmadmin(name host, name admin);
   

   struct [[eosio::table]] withdraws {
      uint64_t id;
      name username;
      name host;
      name status;
      name contract;
      asset    amount;
      std::string address;
      std::string trx_id;
      std::string cancel_reason;
      uint64_t primary_key()const { return id; }
      
      uint64_t byusername() const { return username.value;}
      uint64_t bystatus() const { return status.value;}
      uint64_t byhost() const { return host.value;}
  
   };

   typedef eosio::multi_index<"withdraws"_n, withdraws,
     eosio::indexed_by<"byusername"_n, eosio::const_mem_fun<withdraws, uint64_t, &withdraws::byusername>>,
     eosio::indexed_by<"bystatus"_n, eosio::const_mem_fun<withdraws, uint64_t, &withdraws::bystatus>>,
     eosio::indexed_by<"byhost"_n, eosio::const_mem_fun<withdraws, uint64_t, &withdraws::byhost>>
    > withdraws_index;
   


   struct [[eosio::table]] admins {
      name username;
      
      uint64_t primary_key() const { return username.value;}
      
   };

   typedef eosio::multi_index<"admins"_n, admins> admins_index;
   


    /**
    * @ingroup public_consts 
    * @{ 
    */
    static constexpr eosio::name _me = "withdrawer"_n;   
    static constexpr eosio::name _token = "usdt.token"_n;
    
    /**
    * @}
    */
    
};

  

    