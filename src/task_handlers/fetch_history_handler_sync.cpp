#include "fetch_history_handler_sync.h"

#include "../SyncSingleton.h"

#include "../generate_json.h"

#include "../sync/BlockInfo.h"
#include "../sync/BlockChainReadInterface.h"

bool fetch_history_handler_sync::prepare_params()
{
    BGN_TRY
    {
        CHK_PRM(m_id, "id field not found")
        
        auto params = m_reader.get_params();
        CHK_PRM(params, "params field not found")
        
        std::string addr;
        CHK_PRM(m_reader.get_value(*params, "address", addr), "address field not found")
        CHK_PRM(!addr.empty(), "address is empty")
        CHK_PRM(addr.compare(0, 2, "0x") == 0, "address field incorrect format")
        
        address = addr.c_str();
        
        countTxs = 0;
        auto jValue = this->m_reader.get("countTxs", *params);
        if (jValue)
        {
            std::string tmp;
            CHK_PRM(json_utils::val2str(jValue, tmp), "countTxs field incorrect format")
            countTxs = std::stoull(tmp);
            m_writer.add_param("countTxs", countTxs);
        }
        
        beginTx = 0;
        jValue = this->m_reader.get("beginTx", *params);
        if (jValue)
        {
            std::string tmp;
            CHK_PRM(json_utils::val2str(jValue, tmp), "beginTx field incorrect format")
            beginTx = std::stoull(tmp);
            m_writer.add_param("beginTx", beginTx);
        }
        
        return true;
    }
    END_TRY_RET(false)
}

void fetch_history_handler_sync::executeImpl() {
    CHK_PRM(syncSingleton() != nullptr, "Sync not set");
    const torrent_node_lib::Sync &sync = *syncSingleton();
    
    const std::vector<torrent_node_lib::TransactionInfo> txs = sync.getTxsForAddress(torrent_node_lib::Address(address), true, beginTx, countTxs);
    
    addressesInfoToJson(address, txs, sync.getBlockchain(), 0, false, JsonVersion::V1, m_writer.getDoc());
}
