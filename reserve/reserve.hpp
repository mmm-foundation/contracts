#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <eosio/time.hpp>
#include <eosio/multi_index.hpp>
#include <eosio/contract.hpp>
#include <eosio/crypto.hpp>

static constexpr eosio::name _me = "reserve"_n;

// #define isDebug = TRUE
class [[eosio::contract]] reserve : public eosio::contract {


public:

    reserve( eosio::name receiver, eosio::name code, eosio::datastream<const char*> ds ): eosio::contract(receiver, code, ds)
    {}

    [[eosio::action]] void update();
    void apply(uint64_t receiver, uint64_t code, uint64_t action);
    
    static void reservation(eosio::name contract, eosio::name username, eosio::asset quantity, std::string memo);
        
    
    static uint128_t combine_ids(const uint64_t &x, const uint64_t &y) {
        return (uint128_t{x} << 64) | y;
    };


};
