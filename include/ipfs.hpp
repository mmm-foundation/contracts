namespace eosio {

  struct [[eosio::table]] storage {
    uint64_t id;
    eosio::string name;
    eosio::string address;
    bool encrypted = false;
    eosio::string pkey;
    eosio::string meta;
    uint64_t primary_key() const {return id;}

    EOSLIB_SERIALIZE(struct storage, (id)(name)(address)(encrypted)(pkey)(meta))
  };

  typedef eosio::multi_index<"storage"_n, storage> storage_index;


  struct [[eosio::table]] ipfskeys{
    uint64_t id;
    bool revoked = false;
    eosio::time_point_sec revokedAt;
    eosio::string pkey;
    eosio::string meta;

    uint64_t primary_key() const {return id;}

    EOSLIB_SERIALIZE(struct ipfskeys, (id)(revoked)(revokedAt)(pkey)(meta))
  };

  typedef eosio::multi_index<"ipfskeys"_n, ipfskeys> ipfskeys_index;


  struct [[eosio::table]] userdatacnts
  {
    eosio::name username;
    uint64_t total_sales = 0;
    uint64_t total_buys = 0;
    uint64_t total_disputes = 0;
    uint64_t p_disputes = 0;
    uint64_t n_disputes = 0;

    uint64_t primary_key() const {return username.value;}

    EOSLIB_SERIALIZE(struct userdatacnts, (username)(total_sales)(total_buys)(total_disputes)(p_disputes)(n_disputes))

  };

  typedef eosio::multi_index<"userdatacnts"_n, userdatacnts> userdatacounts_index;



  struct [[eosio::table]] orbdata {
    uint64_t id;
    eosio::string data;
    eosio::name root_token_contract;
    eosio::asset amount;
    uint16_t sales_count = 0;
    uint16_t dispute_count = 0;
    uint64_t p_disputes = 0;
    uint64_t n_disputes = 0;
    bool removed = false;
    eosio::string meta;

    uint64_t primary_key() const {return id;}

    EOSLIB_SERIALIZE(struct orbdata, (id)(data)(root_token_contract)(amount)(sales_count)(dispute_count)(p_disputes)(n_disputes)(removed)(meta))
  };

  typedef eosio::multi_index<"orbdata"_n, orbdata> orbdata_index;




  struct [[eosio::table]] mydataordrs {
    eosio::name owner;
    uint64_t order_id;

    uint128_t primary_key() const { return combine_ids(owner.value, order_id); }

    EOSLIB_SERIALIZE(struct mydataordrs, (owner)(order_id))
  };

  typedef eosio::multi_index< "mydataordrs"_n, mydataordrs> mydataordrs_index;




  struct [[eosio::table]] dataorders {
    uint64_t id;  
    uint64_t orbdata_id;   
    eosio::time_point_sec opened_at;
    eosio::time_point_sec expired_at;
    eosio::name owner;
    eosio::name buyer;
    eosio::name curator;
    eosio::asset locked_amount;

    bool approved = false;
    eosio::string key;

    bool dispute = false;
    eosio::string meta;

    uint64_t primary_key() const {return id;}
    uint128_t buyerandid() const {return combine_ids(buyer.value, orbdata_id);}

    EOSLIB_SERIALIZE(struct dataorders, (id)(orbdata_id)(opened_at)(expired_at)(owner)(buyer)(curator)(locked_amount)(approved)
      (key)(dispute)(meta))
  };

  typedef eosio::multi_index<"dataorders"_n, dataorders,
      indexed_by<"buyerandid"_n, const_mem_fun<dataorders, uint128_t, 
                              &dataorders::buyerandid>>
  > dataorders_index;


  
  struct [[eosio::action]] selldata
  {
    eosio::name username;
    uint64_t id;
    eosio::string data;
    eosio::name root_token_contract;
    eosio::asset amount;  

    EOSLIB_SERIALIZE(selldata, (username)(id)(data)(root_token_contract)(amount))
  };

  struct [[eosio::action]] dataapprove
  {
    eosio::name username;
    eosio::name owner;
    eosio::name who;
    uint64_t order_id;
    eosio::string message; 

    EOSLIB_SERIALIZE(dataapprove, (username)(owner)(who)(order_id)(message))
  };




  struct [[eosio::action]] datadispute
  {
    eosio::name username;
    eosio::string data;
    eosio::name root_token_contract;
    eosio::asset amount;  

    EOSLIB_SERIALIZE(datadispute, (username)(data)(root_token_contract)(amount))
  };

  struct [[eosio::action]] datarelease
  {
    eosio::name username;
    eosio::string data;
    eosio::name root_token_contract;
    eosio::asset amount;  

    EOSLIB_SERIALIZE(datarelease, (username)(data)(root_token_contract)(amount))
  };

  struct [[eosio::action]] setstorage {
    eosio::name username;
    uint64_t id;
    eosio::string name;
    eosio::string address;
    
    EOSLIB_SERIALIZE(setstorage, (username)(id)(name)(address))
  };


  struct [[eosio::action]] removeroute {
    eosio::name username;
    uint64_t id;
    
    EOSLIB_SERIALIZE(removeroute, (username)(id))
  };


  struct [[eosio::action]] setipfskey {
    eosio::name username;
    eosio::string pkey;
    eosio::string meta;
    
    EOSLIB_SERIALIZE(setipfskey, (username)(pkey)(meta))
  };


}

