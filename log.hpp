#include <eosiolib/multi_index.hpp>
#include <eosiolib/contract.hpp>
#include "hash64.hpp"
#include <eosiolib/binary_extension.hpp> 
// #define isDebug = TRUE
class [[eosio::contract]] log : public eosio::contract {

public:
    log( eosio::name receiver, eosio::name code, eosio::datastream<const char*> ds ): eosio::contract(receiver, code, ds)
    {}
    [[eosio::action]] void post(eosio::name owner, eosio::name author, std::string permlink, eosio::name parent_author, std::string parent_permlink, std::string title, std::string body, std::string meta, bool comments_is_enabled, int64_t priority, bool is_encrypted, std::string public_key);
    [[eosio::action]] void del(eosio::name owner, eosio::name author, std::string permlink);
    [[eosio::action]] void selldata(eosio::name owner, uint64_t id, eosio::name root_token_contract, eosio::asset amount);
    [[eosio::action]] void approve(eosio::name owner, eosio::name who, uint64_t order_id, std::string message);
    [[eosio::action]] void dispute(eosio::name owner, eosio::name buyer, uint64_t order_id);
    
    static void buydata(eosio::name buyer, eosio::name owner, uint64_t id, eosio::asset quantity, uint64_t code);
    
    void apply(uint64_t receiver, uint64_t code, uint64_t action);
    
    static constexpr eosio::name _self = "log"_n;
    static constexpr eosio::name _curator = "curator"_n;
        
    #ifdef isDebug
        static const uint64_t _DATA_ORDER_EXPIRATION = 10; 
    #else 
        static const uint64_t _DATA_ORDER_EXPIRATION = 86400; //86400

    #endif

    static const uint64_t MIN_PERMLINK_LENGTH = 3;
    static const uint64_t MAX_PERMLINK_LENGTH = 256;
    
    
    


    static uint128_t combine_ids(const uint64_t &x, const uint64_t &y) {
        return (uint128_t{x} << 64) | y;
    };

    struct [[eosio::table]] comments {
        uint64_t hash;
        uint64_t parent_hash;
        eosio::name author;
        std::string permlink;
        eosio::name parent_author;
        std::string parent_permlink;
        
        std::string body;
        std::string title;
        std::string meta;
        
        eosio::time_point_sec created;
        eosio::time_point_sec last_update;
        
        bool is_encrypted = false;
        std::string public_key;

        bool comments_is_enabled = false;

        int64_t priority = 0;
        
        uint64_t primary_key() const {return hash;}
        
        EOSLIB_SERIALIZE(comments, (hash)(parent_hash)(author)(permlink)(parent_author)(parent_permlink)(body)(title)(meta)(created)(last_update)(is_encrypted)(public_key)(comments_is_enabled)(priority))
    };

    typedef eosio::multi_index<"comments"_n, comments> comments_index;





    struct [[eosio::table]] dataorders {
        uint64_t id;  
        uint64_t data_id;   
        eosio::time_point_sec opened_at;
        eosio::time_point_sec expired_at;

        eosio::name owner;
        eosio::name buyer;
        eosio::name curator;
        eosio::asset locked_amount;

        bool approved = false;
        std::string keys;

        bool dispute = false;
        std::string meta;

        uint64_t primary_key() const {return id;}
        uint128_t bybuyerandid() const {return combine_ids(buyer.value, data_id);}

        EOSLIB_SERIALIZE(struct dataorders, (id)(data_id)(opened_at)(expired_at)(owner)(buyer)(curator)(locked_amount)(approved)
          (keys)(dispute)(meta))
      };

      typedef eosio::multi_index<"dataorders"_n, dataorders,
           eosio::indexed_by< "bybuyerandid"_n, eosio::const_mem_fun<dataorders, uint128_t, 
                                  &dataorders::bybuyerandid>>
      > dataorders_index;

    



    struct [[eosio::table]] mydataorders {
        eosio::name owner;
        uint64_t order_id;

        uint128_t primary_key() const { return combine_ids(owner.value, order_id); }

        EOSLIB_SERIALIZE(struct mydataorders, (owner)(order_id))
    };

    typedef eosio::multi_index< "mydataorders"_n, mydataorders> mydataorders_index;





  struct [[eosio::table]] userdatacnts
  {
    eosio::name username;
    uint16_t total_sales = 0;
    uint16_t total_buys = 0;
    uint16_t total_disputes = 0;
    uint16_t p_disputes = 0;
    uint16_t n_disputes = 0;

    uint64_t primary_key() const {return username.value;}

    EOSLIB_SERIALIZE(struct userdatacnts, (username)(total_sales)(total_buys)(total_disputes)(p_disputes)(n_disputes))

  };

  typedef eosio::multi_index<"userdatacnts"_n, userdatacnts> userdatacounts_index;



  struct [[eosio::table]] dispdisq {
    uint64_t order_id;
    
    eosio::name author;
    std::string message;

    uint64_t primary_key() const {return order_id;}

    EOSLIB_SERIALIZE(struct dispdisq, (order_id)(author)(message))
  };

  typedef eosio::multi_index<"dispdisq"_n, dispdisq> dispdisq_index;




  struct [[eosio::table]] dataonsale {
    uint64_t id;
    eosio::name root_token_contract;
    eosio::asset amount;
    uint16_t sales_count = 0;
    uint16_t dispute_count = 0;
    uint16_t p_disputes = 0;
    uint16_t n_disputes = 0;
    bool removed = false;
    std::string meta;

    uint64_t primary_key() const {return id;}

    EOSLIB_SERIALIZE(struct dataonsale, (id)(root_token_contract)(amount)(sales_count)(dispute_count)(p_disputes)(n_disputes)(removed)(meta))
  };

  typedef eosio::multi_index<"dataonsale"_n, dataonsale> dataonsale_index;

 
};
