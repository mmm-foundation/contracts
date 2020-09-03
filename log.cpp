#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/time.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/system.hpp>
#include "log.hpp"

using namespace eosio;

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


  [[eosio::action]] void log::post(eosio::name owner, eosio::name author, std::string permlink, eosio::name parent_author, std::string parent_permlink, std::string title, std::string body, std::string meta, bool comments_is_enabled, int64_t priority, bool is_encrypted, std::string public_key){
    require_auth(author);
    
    comments_index comments (_self, owner.value);
    validate_permlink(permlink);
    
    auto hash = hash64(permlink);
    auto comment = comments.find(hash);
    
    eosio::check(title.length() < 140, "The maximum of title length is a 140 symbols");
    auto parent_hash = hash64(parent_permlink);
    print("Parent hash", parent_hash);

    // if ((parent_permlink != "") && (parent_author.value != 0)){        
    //permlink == "" - is Manifest
    // 
    if (permlink != "") {
        comments_index parent_comments(_self, owner.value);
        auto parent_comment = parent_comments.find(parent_hash);
        
        // if (parent_permlink != "")
        // eosio::check(parent_comment != parent_comments.end(), "Parent comment is not found");
    
        if (owner.value != author.value){
          eosio::check(parent_comment -> comments_is_enabled == true, "Comments is disabled");

        }

    } else {
      
        eosio::check(owner.value == author.value, "Only owner can modify community manifest");
        eosio::check(parent_author.value == author.value,"On modify community manifest parent author should be same as a author");
      
    }
    // } else if(parent_author.value == 0){
      // eosio::check(owner == author, "Only owner can comment his first line");
    // }
    

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


  [[eosio::action]] void log::del(eosio::name owner, eosio::name author, std::string permlink){
  
    require_auth(author);

    comments_index comments (_self, owner.value);
    auto hash = hash64(permlink);
    auto comment = comments.find(hash);
    eosio::check(comment != comments.end(), "Comment is not found");
    eosio::check(comment->permlink == permlink, "Wrong permlink");
    eosio::check(comment->author == author, "Wrong author");
    
    comments.erase(comment);
  }


  [[eosio::action]] void log::approve(eosio::name owner, eosio::name who, uint64_t order_id, std::string message){
    require_auth(who);

    dataorders_index data_orders(_self, owner.value);
    auto order = data_orders.find(order_id);
    eosio::check(order != data_orders.end(), "Order is not found");
    
    dataonsale_index dataonsale(_self, owner.value);
    auto data = dataonsale.find(order-> data_id);
    eosio::check(data != dataonsale.end(), "Data unit is not found");

    if ((who == owner) && (who == order -> owner)){

      data_orders.modify(order, who, [&](auto &oo){
        oo.approved = true;
        oo.keys = message;
        oo.expired_at = eosio::time_point_sec(now() + _DATA_ORDER_EXPIRATION);
      });

    } else if (who == order -> buyer){

      eosio::check(order -> approved == true, "Cannot finish order before approve from owner");

      action(
            permission_level{ _self, "active"_n },
            data->root_token_contract, "transfer"_n,
            std::make_tuple( _self, order->owner, order -> locked_amount, std::string("Order is closed positive.")) 
      ).send();

      dataonsale.modify(data, who, [&](auto &d){
        d.sales_count = data -> sales_count + 1;
      });

      data_orders.erase(order);

      mydataorders_index mydataorders (_self, (order->buyer).value);


      auto seller_with_order_id_idx = combine_ids((order->owner).value, order-> id);
      auto sellers_with_my_order = mydataorders.find(seller_with_order_id_idx);

      mydataorders.erase(sellers_with_my_order);


      userdatacounts_index usercounts(_self, _self.value);

      auto buyercounts = usercounts.find((order -> buyer).value);
      auto ownercounts = usercounts.find((order -> owner).value);

      if (buyercounts == usercounts.end()){
        
        usercounts.emplace(who, [&](auto &uc){
          uc.username = order->buyer;
          uc.total_buys = 1;
        });

      } else {

        usercounts.modify(buyercounts, who, [&](auto &uc){
          uc.total_buys = buyercounts -> total_buys + 1;
        });

      }

      if (ownercounts == usercounts.end()){
        
        usercounts.emplace(who, [&](auto &uc){
          uc.username = order-> owner;
          uc.total_sales = 1;
        });

      } else {
        
        usercounts.modify(ownercounts, who, [&](auto &uc){
          uc.total_sales = ownercounts -> total_sales + 1;
        });

      }


    } 


  }




  
  [[eosio::action]] void log::dispute(eosio::name owner, eosio::name buyer, uint64_t order_id) {
      require_auth(buyer);

      dataorders_index data_orders(_self, owner.value);
    
      auto order = data_orders.find(order_id);

      eosio::check(order -> dispute == false, "Dispute is already open");
      eosio::check(order != data_orders.end(), "Order is not exist");
      eosio::check(order -> buyer == buyer, "Wrong buyer for current order");
      // eosio::check(order -> expired_at <= eosio::current_time_point(), "Dispute can be created only after order expiration time");


      data_orders.modify(order, buyer, [&](auto &d){
        d.dispute = true;
        d.curator = _curator;
      });

      dispdisq_index dispdisq(_self, owner.value);
      
      dispdisq.emplace(buyer, [&](auto &d){
        d.order_id = order_id;
        d.author = _self;
        d.message = "Dispute is open";
      });

  }

  // [[eosio::action]] void log::release(eosio::name owner, eoiso::name buyer, uint64_t data_id) {
    
  // }

  // [[eosio::action]] void log::addcommtodis(eosio::name owner, eoiso::name buyer, uint64_t data_id) {
    
  // }




  [[eosio::action]] void log::selldata(eosio::name owner, uint64_t data_id, eosio::name root_token_contract, eosio::asset amount){
    require_auth(owner);

    comments_index comments (_self, owner.value);
    auto comment = comments.find(data_id);
    eosio::check(comment != comments.end(), "Comment with current ID is not found");
    eosio::check((comment -> author).value == owner.value, "You can sell only own data");

    dataonsale_index dataonsale(_self, owner.value);

    auto data = dataonsale.find(data_id);
    
    if (data == dataonsale.end()){
      dataonsale.emplace(owner, [&](auto &d){
        d.id = data_id;
        d.root_token_contract = root_token_contract;
        d.amount = amount;
      });
    } else {
      dataonsale.modify(data, owner, [&](auto &d){
        d.root_token_contract = root_token_contract;
        d.amount = amount;
      });
    }
  }

  void log::buydata(eosio::name buyer, eosio::name owner, uint64_t data_id, eosio::asset quantity, uint64_t code){
    require_auth(buyer);

    dataonsale_index dataonsale(_self, owner.value);
    auto data = dataonsale.find(data_id);


    eosio::check(data != dataonsale.end(), "Data object is not found");
    eosio::check((data -> root_token_contract).value == code, "Wrong root token contract");
    eosio::check((data -> amount).symbol == quantity.symbol, "Wrong symbol for this data unit");
    eosio::check(data -> amount <= quantity, "Not enought quantity of tokens for buy this data unit");
    

    dataorders_index data_orders(_self, owner.value);

    auto buyers_with_data_id = data_orders.template get_index<"bybuyerandid"_n>();

    auto buyers_with_data_id_ids = combine_ids(buyer.value, data->id);
    
    auto exist = buyers_with_data_id.find(buyers_with_data_id_ids);

    eosio::check(exist == buyers_with_data_id.end(), "Order from buyer already exist for current data unit");


    auto order_id = data_orders.available_primary_key();
    
    data_orders.emplace(_self, [&](auto &oo){
      oo.id = order_id;
      oo.data_id = data_id;
      oo.opened_at = eosio::time_point_sec(now());
      oo.owner = owner;
      oo.buyer = buyer;
      oo.locked_amount = quantity;
      oo.expired_at = eosio::time_point_sec(-1);
    });

    mydataorders_index mydataorders_index (_self, buyer.value);

    auto seller_with_order_id_idx = combine_ids(owner.value, data_id);

    mydataorders_index.emplace(_self, [&](auto &ml){
      ml.owner = owner;
      ml.order_id = order_id;
    });

  }


extern "C" {
   
   /// The apply method implements the dispatch of events to this contract
   void apply( uint64_t receiver, uint64_t code, uint64_t action ) {
        if (code == log::_self.value) {
          if (action == "post"_n.value){
            execute_action(name(receiver), name(code), &log::post);
          } else if (action == "del"_n.value){
            execute_action(name(receiver), name(code), &log::del);
          } else if (action == "selldata"_n.value){
            execute_action(name(receiver), name(code), &log::selldata);
          } else if (action == "approve"_n.value){
            execute_action(name(receiver), name(code), &log::approve);
          } else if (action == "dispute"_n.value){
            execute_action(name(receiver), name(code), &log::dispute);
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

            if (op.to == log::_self){
              auto delimeter = op.memo.find('-');
              std::string string_owner = op.memo.substr(0, delimeter);
              std::string string_id = op.memo.substr(delimeter+1, op.memo.length());
              
              eosio::name owner = eosio::name(string_owner.c_str());
              uint128_t id = std::atoll(string_id.c_str());    
              if (owner.value != 0)
                log::buydata(op.from, owner, id, op.quantity, code);
            }
          }
        }
  };
};
