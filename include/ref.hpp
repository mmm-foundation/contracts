namespace eosio {

    struct [[eosio::table, eosio::contract("unicore")]] refbalances{
        uint64_t id;
        eosio::name host;
        eosio::time_point_sec timepoint_sec;
        eosio::asset refs_amount;
        eosio::asset win_amount;
        eosio::asset amount;
        eosio::name from;
        uint64_t level;
        uint128_t segments;
        uint64_t primary_key() const {return id;}

        EOSLIB_SERIALIZE(refbalances, (id)(host)(timepoint_sec)(refs_amount)(win_amount)(amount)(from)(level)(segments))
    };

    typedef eosio::multi_index<"refbalances"_n, refbalances> refbalances_index;


    struct [[eosio::table, eosio::contract("unicore")]] rstat{
        eosio::name host;
        eosio::asset withdrawed;
        
        uint64_t primary_key() const {return host.value;}

        EOSLIB_SERIALIZE(rstat, (host)(withdrawed))
    };

    typedef eosio::multi_index<"rstat"_n, rstat> rstat_index;



}