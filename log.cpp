#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/time.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/system.hpp>
#include "log.hpp"

using namespace eosio;
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


  void validate_permlink( const std::string& permlink )
        {
           eosio::check( permlink.size() < log::MAX_PERMLINK_LENGTH, "Permlink is not a valid size." );
           if ( permlink.size() > 0 )
             for( auto c : permlink )
             {
                switch( c )
                {
                   case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i':
                   case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p': case 'q': case 'r':
                   case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z': case '0':
                   case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
                   case '-':
                      break;
                   default:
                      eosio::check( false, "Invalid permlink character");
                }
             }
        }


  /**
   * @brief      Метод публикации поста
   * @auth author
   * @details Метод публикует пост от автора в пространстве памяти хоста. 
   * Каждый пользователь может использовать своё пространство в памяти для хранения постов. 
   * @ingroup public_actions
   * @param[in]  owner                Пространство в памяти для хранения поста (хост)
   * @param[in]  author               Автор поста
   * @param[in]  permlink             Строковая прямая ссылка
   * @param[in]  parent_author        Автор родительского поста (не обязательно)
   * @param[in]  parent_permlink      Строковая прямая ссылка родительского поста (не обязательно)
   * @param[in]  title                Заголовок поста
   * @param[in]  body                 Тело поста
   * @param[in]  meta                 Мета-данные поста
   * @param[in]  comments_is_enabled  Индикатор режима комментирования (вкл/вкл)
   * @param[in]  priority             Приоритет поста
   * @param[in]  is_encrypted         Указатель шифрованного поста
   * @param[in]  public_key           Публичный ключ для дешифровки поста
   */
  [[eosio::action]] void log::post(eosio::name owner, eosio::name author, std::string permlink, eosio::name parent_author, std::string parent_permlink, std::string title, std::string body, std::string meta, bool comments_is_enabled, int64_t priority, bool is_encrypted, std::string public_key){
    require_auth(author);
    
    comments_index comments (_me, owner.value);
    validate_permlink(permlink);
    
    auto hash = hash64(permlink);
    auto comment = comments.find(hash);
    
    eosio::check(title.length() < 140, "The maximum of title length is a 140 symbols");
    auto parent_hash = hash64(parent_permlink);
    
    if (permlink != "") {
        comments_index parent_comments(_me, owner.value);
        auto parent_comment = parent_comments.find(parent_hash);
        
    
        if (owner.value != author.value){
    
        }

    } else {
      
        eosio::check(owner.value == author.value, "Only owner can modify community manifest");
        eosio::check(parent_author.value == author.value,"On modify community manifest parent author should be same as a author");
      
    }
    

    if (comment == comments.end()){

      comments.emplace(author, [&](auto &c){
        c.hash = hash;
        c.parent_hash = parent_hash;
        c.author = author;
        c.permlink = permlink;
        c.parent_author = parent_author;
        c.parent_permlink = parent_permlink;
        c.body = body;
        c.title = title;
        c.meta = meta;
        c.created = eosio::time_point_sec(now());
        c.last_update = c.created;
        c.comments_is_enabled = comments_is_enabled;
        c.priority = priority;
        c.is_encrypted = is_encrypted;
        c.public_key = public_key;
        
      });

    }
     else {

      comments.modify(comment, author, [&](auto &c){
        c.body = body;
        c.title = title;
        c.meta = meta;
        c.last_update = eosio::time_point_sec(now());
        c.comments_is_enabled = comments_is_enabled;
        c.priority = priority;
        c.is_encrypted = is_encrypted;
        c.public_key = public_key;
      });
    
    }

  }


  /**
   * @brief      Метод удаления поста
   * @auth author
   * @ingroup public_actions
   * @details Удаляет пост из оперативной памяти
   * @param[in]  owner     Пространство памяти хранения поста
   * @param[in]  author    Автор поста
   * @param[in]  permlink  Прямая ссылка на пост
   */

  [[eosio::action]] void log::del(eosio::name owner, eosio::name author, std::string permlink){
  
    require_auth(author);

    comments_index comments (_me, owner.value);
    auto hash = hash64(permlink);
    auto comment = comments.find(hash);
    eosio::check(comment != comments.end(), "Comment is not found");
    eosio::check(comment->permlink == permlink, "Wrong permlink");
    eosio::check(comment->author == author, "Wrong author");
    
    comments.erase(comment);
  }



extern "C" {
   
   /// The apply method implements the dispatch of events to this contract
   void apply( uint64_t receiver, uint64_t code, uint64_t action ) {
        if (code == log::_me.value) {
          if (action == "post"_n.value){
            execute_action(name(receiver), name(code), &log::post);
          } else if (action == "del"_n.value){
            execute_action(name(receiver), name(code), &log::del);
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

            if (op.to == log::_me){
              //do something on incoming transfer
            }
          }
        }
  };
};
