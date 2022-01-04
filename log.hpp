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
    
    void apply(uint64_t receiver, uint64_t code, uint64_t action);
    

    /**
    * @ingroup public_consts 
    * @{ 
    */

    static constexpr eosio::name _me = "log"_n;         /*!< собственное имя аккаунта контракта */
    static const uint64_t MIN_PERMLINK_LENGTH = 3;      /*!< минимальное количество символов в прямой ссылке */
    static const uint64_t MAX_PERMLINK_LENGTH = 256;    /*!< максимальное количество символов в прямой ссылке */
    /**
    * @}
    */
    
    


    static uint128_t combine_ids(const uint64_t &x, const uint64_t &y) {
        return (uint128_t{x} << 64) | y;
    };


    /**
     * @brief      Таблица хранения постов пользователей
     * @contract _me
     * @scope owner
     * @table comments
     * @ingroup public_tables
     */
    struct [[eosio::table]] comments {
        uint64_t hash;                                  /*!< хэш-сумма поста от прямой ссылки */
        uint64_t parent_hash;                           /*!< хэш-сумма родительского поста от прямой ссылки */
        eosio::name author;                             /*!< автор поста */
        std::string permlink;                           /*!< прямая ссылка */
        eosio::name parent_author;                      /*!< автор родительского поста */
        std::string parent_permlink;                    /*!< прямая ссылка родительского поста */
        
        std::string body;                               /*!< тело поста */
        std::string title;                              /*!< заголовок поста */
        std::string meta;                               /*!< мета-данные поста */
        
        eosio::time_point_sec created;                  /*!< дата создания поста */
        eosio::time_point_sec last_update;              /*!< дата последнего обновления поста */
        
        bool is_encrypted = false;                      /*!< индикатор шифрования поста */
        std::string public_key;                         /*!< публичный ключ шифрования поста */

        bool comments_is_enabled = false;               /*!< индикатор подключенных комментариев (вкл / выкл) */

        int64_t priority = 0;                           /*!< приоритет поста */
        
        uint64_t primary_key() const {return hash;}     /*!< return hash - primary_key */
        
        EOSLIB_SERIALIZE(comments, (hash)(parent_hash)(author)(permlink)(parent_author)(parent_permlink)(body)(title)(meta)(created)(last_update)(is_encrypted)(public_key)(comments_is_enabled)(priority))
    };

    typedef eosio::multi_index<"comments"_n, comments> comments_index;


 
};
