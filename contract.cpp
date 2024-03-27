#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/action.hpp>
#include <eosio/crypto.hpp>
#include <eosio/serialize.hpp>
#include <eosio/permission.hpp>
#include <eosio/transaction.hpp>
#include <vector>
#include <array>

#define RAM_RECOVER_ACCOUNT "ram.recover"_n
using namespace eosio;

class [[eosio::contract("eos_sale")]] eos_sale : public contract {
public:
    using contract::contract;

    eos_sale(name receiver, name code, datastream<const char*> ds)
        : contract(receiver, code, ds),
          sold_accounts(receiver, receiver.value) {
        // 初始化sold_accounts表
        auto itr = sold_accounts.begin();
        if (itr == sold_accounts.end()) {
            sold_accounts.emplace(get_self(), [&](auto& row) {
                row.last_sold_id = 0;
                row.price = asset(2048, symbol("WRAM", 0)); // 默认价格为 2048 WRAM
            });
        }
    }

    [[eosio::on_notify("eosio.token::transfer")]]
    void on_transfer(name from, name to, asset quantity, std::string memo) {
        if (from == get_self() || to != get_self() || quantity.symbol != symbol("EOS", 4)) {
            return; // 只处理接收到EOS的转账
        }

        if (memo.size() < 4 || memo.substr(0, 4) != "buy-") {

            return;
        }

        check(quantity.amount > 0, "Quantity must be positive");

        // 卖出 EOS 并获取获得的 RAM 数量
        uint64_t bytes_obtained = sell_eos_for_ram(get_self(), quantity);

        purchase(from, bytes_obtained, memo);
    }

    [[eosio::on_notify("eosio::ramtransfer")]]
    void on_ramtransfer(name from, name to, int64_t bytes, std::string memo) {
        if (from == get_self() || to != get_self()) {
            return; // 只处理接收到RAM的转账
        }

        purchase(from, bytes, memo);
    }

    [[eosio::on_notify("eosio.wram::transfer")]]
    void on_wram_transfer(name from, name to, asset quantity, std::string memo) {
        if (from == get_self() || to != get_self() || quantity.symbol != symbol("WRAM", 0)) {
            return; // 只处理接收到WRAM的转账
        }

        if (memo.size() < 4 || memo.substr(0, 4) != "buy-") {

            return;
        }

        check(quantity.amount > 0, "Quantity must be positive");

        // 把WRAM换成RAM
        action(
            permission_level{get_self(), "active"_n},
            "eosio.wram"_n, 
            "transfer"_n,
            std::make_tuple(get_self(), "eosio.wram"_n, quantity, std::string(""))
        ).send();

        // 直接使用接收到的 WRAM 数量进行购买处理
        purchase(from, quantity.amount, memo);
    }

    [[eosio::action]]
    void updaccounts(const std::vector<name>& new_accounts) {
        // 确保只有合约拥有者可以调用此action
        require_auth(get_self());

        accounts_table accounts(get_self(), get_self().value);
        sold_accounts_table sold_accounts(get_self(), get_self().value);

        auto last_sold_itr = sold_accounts.begin();
        uint64_t last_sold_id = last_sold_itr != sold_accounts.end() ? last_sold_itr->last_sold_id : 0;

        uint64_t max_purchase_quantity = last_sold_itr->max_purchase_quantity;
        last_sold_id = last_sold_id > max_purchase_quantity ? last_sold_id - max_purchase_quantity : 0;

        // 清除已售出的账户，但保留最后售出的1000条记录
        auto account_itr = accounts.begin();
        while (account_itr != accounts.end() && account_itr->id < last_sold_id) {

            account_itr = accounts.erase(account_itr);
        }

        // 添加新的可售账号列表
        for (const auto& acc : new_accounts) {
            accounts.emplace(get_self(), [&](auto& a) {
                auto id = accounts.available_primary_key();
                a.id = id == 0 ? 1 : id;
                a.eos_account = acc;
            });
        }
    }

    [[eosio::action]]
    void setprice(asset new_price) {
        require_auth(get_self());
        check(new_price.amount > 0 && new_price.symbol == symbol("WRAM", 0), "Invalid price");

        auto itr = sold_accounts.begin();
        check(itr != sold_accounts.end(), "Price setting not initialized");
        sold_accounts.modify(itr, get_self(), [&](auto& s) {

            s.price = new_price;
        });
    }

    // 添加设置最大购买数量的 action
    [[eosio::action]]
    void setmaxqty(uint64_t max_qty) {
        require_auth(get_self());

        auto itr = sold_accounts.begin();
        check(itr != sold_accounts.end(), "Sold accounts table not initialized");
        sold_accounts.modify(itr, get_self(), [&](auto& s) {
            s.max_purchase_quantity = max_qty;
        });
    }

    // 添加清除 accounts 表的 action
    [[eosio::action]]
    void clearaccts() {
        require_auth(get_self());

        accounts_table accounts(get_self(), get_self().value);

        // 遍历并删除 accounts 表中的所有记录
        auto itr = accounts.begin();
        while (itr != accounts.end()) {
            itr = accounts.erase(itr);
        }
    }

    // 添加清除 sales 表的 action
    [[eosio::action]]
    void clearsales() {
        require_auth(get_self());

        sales_table sales(get_self(), get_self().value);

        // 遍历并删除 sales 表中的所有记录
        auto itr = sales.begin();
        while (itr != sales.end()) {
            itr = sales.erase(itr);
        }
    }

    // 添加重置 sold_accounts 表的 last_sold_id 的 action
    [[eosio::action]]
    void resetsold() {
        require_auth(get_self());

        sold_accounts_table sold_accounts(get_self(), get_self().value);

        auto itr = sold_accounts.begin();
        if (itr != sold_accounts.end()) {
            sold_accounts.modify(itr, get_self(), [&](auto& row) {
                row.last_sold_id = 0;
            });
        }
    }

private:
    void purchase(name buyer, uint64_t bytes, std::string memo){

        if (memo.size() < 4 || memo.substr(0, 4) != "buy-") {

            return;
        }

        check(bytes > 0, "Bytes must be positive");

        // 调用process_purchase函数处理购买逻辑
        process_purchase(buyer, memo.substr(4), asset(bytes, symbol("WRAM", 0)));
    }

    void process_purchase(name buyer, std::string memo, asset quantity) {

        auto sold_account_itr = sold_accounts.begin();
        check(sold_account_itr != sold_accounts.end(), "Sold accounts table not initialized");

        auto price = sold_account_itr->price;
        uint64_t accounts_to_sell = quantity.amount / price.amount;

        check(accounts_to_sell > 0, "Amount is not enough to buy an account");

        // 检查购买数量是否超过最大购买数量
        check(accounts_to_sell <= sold_account_itr->max_purchase_quantity, "Cannot purchase more than the maximum allowed quantity");

        auto pubkey = is_valid_pubkey(memo); // 校验公钥并获取公钥对象

        accounts_table accounts(get_self(), get_self().value);
        sold_accounts_table sold_accounts(get_self(), get_self().value);
        sales_table sales(get_self(), get_self().value);

        std::vector<uint64_t> accounts_to_update;
        auto last_sold_itr = sold_accounts.begin();
        if (last_sold_itr != sold_accounts.end()) {
            auto account_itr = accounts.find(last_sold_itr->last_sold_id + 1);
            while (account_itr != accounts.end() && accounts_to_update.size() < accounts_to_sell) {
                accounts_to_update.push_back(account_itr->id);
                ++account_itr;
            }
        }

        check(accounts_to_update.size() == accounts_to_sell, "Not enough unsold accounts available");

        for (const auto& acc_id : accounts_to_update) {
            auto acc_itr = accounts.find(acc_id);
            if (acc_itr != accounts.end()) {

                action(
                    permission_level{acc_itr->eos_account, "owner"_n},
                    "eosio"_n, 
                    "updateauth"_n,
                    std::make_tuple(
                        acc_itr->eos_account,
                        "owner"_n,
                        ""_n,
                        authority{
                            1, // threshold
                            {key_weight{pubkey, 1}}, // keys
                            {}, // accounts
                            {}  // waits
                        }
                    )
                ).send();

                action(
                    permission_level{acc_itr->eos_account, "owner"_n},
                    "eosio"_n, 
                    "updateauth"_n,
                    std::make_tuple(
                        acc_itr->eos_account,
                        "active"_n,
                        "owner"_n,
                        authority{
                            1, // threshold
                            {key_weight{pubkey, 1}}, // keys
                            {}, // accounts
                            {}  // waits
                        }
                    )
                ).send();

            }
        }

        // 更新最后售出的账户ID
        if (!accounts_to_update.empty()) {
            auto last_sold_id = accounts_to_update.back();
            if (last_sold_itr != sold_accounts.end()) {
                sold_accounts.modify(last_sold_itr, get_self(), [&](auto& s) {
                    s.last_sold_id = last_sold_id;
                });
            } else {
                sold_accounts.emplace(get_self(), [&](auto& s) {
                    s.last_sold_id = last_sold_id;
                });
            }
        }

        uint64_t total_amount = (accounts_to_sell * price.amount);
        uint64_t remaining = quantity.amount - total_amount;
        asset remaining_asset = asset(remaining, quantity.symbol);

        // 记录销售信息
        auto tx_id = get_trx_id();
        sales.emplace(get_self(), [&](auto& s) {
            s.id = sales.available_primary_key();
            s.buyer = buyer;
            s.purchase_time = current_time_point();
            s.tx_id = tx_id;
            s.quantity = accounts_to_sell;
            s.start_id = accounts_to_update.front();
            s.pubkey = memo;
            s.purchase_amount = quantity; // 记录购买金额或RAM
            s.remaining_amount = remaining_asset;
        });

        // 资金转移到指定账户
        action(
            permission_level{get_self(), "active"_n},
            "eosio"_n, 
            "ramtransfer"_n,
            std::make_tuple(get_self(), RAM_RECOVER_ACCOUNT, total_amount, std::string(""))
        ).send();

        // 返还剩余RAM
        if (remaining > 0) {
            action(
                permission_level{get_self(), "active"_n},
                "eosio"_n, 
                "ramtransfer"_n,
                std::make_tuple(get_self(), buyer, remaining, std::string("Remaining RAM after purchase"))
            ).send();
        }
    }

    struct key_weight {
        eosio::public_key key;
        uint16_t weight;

        key_weight(){};
        key_weight(eosio::public_key p, uint16_t w):key(p),weight(w){};

        // 序列化
        EOSLIB_SERIALIZE(key_weight, (key)(weight))
    };

    struct permission_level_weight {
        permission_level permission;
        uint16_t weight;

        permission_level_weight(){};

        EOSLIB_SERIALIZE(permission_level_weight, (permission)(weight))
    };

    struct wait_weight {
        uint32_t wait_sec;
        uint16_t weight;

        wait_weight(){};

        EOSLIB_SERIALIZE(wait_weight, (wait_sec)(weight))
    };

    struct authority {
        uint32_t threshold;
        std::vector<key_weight> keys;
        std::vector<permission_level_weight> accounts;
        std::vector<wait_weight> waits;

        authority(){};
        authority(uint32_t t, std::vector<key_weight> k, std::vector<permission_level_weight> a, std::vector<wait_weight> w):threshold(t),keys(k),accounts(a),waits(w){};

        // 序列化
        EOSLIB_SERIALIZE(authority, (threshold)(keys)(accounts)(waits))
    };

    struct [[eosio::table]] account {
        uint64_t    id;
        name        eos_account;

        uint64_t primary_key() const { return id; }
    };

    struct [[eosio::table]] sold_account {
        uint64_t last_sold_id;
        asset    price; // 每个账户的单价（RAM）
        uint64_t max_purchase_quantity = 1000; // 默认最大购买数量为 1000
        uint64_t primary_key() const { return 0; } // Singleton table
    };

    // 修改 sale 结构体，添加 buyer 作为二级索引
    struct [[eosio::table]] sale {
        uint64_t      id;
        name          buyer;
        time_point    purchase_time;
        checksum256   tx_id;
        uint64_t      quantity;
        uint64_t      start_id;
        std::string   pubkey;
        asset         purchase_amount; // 购买金额或RAM
        asset         remaining_amount; // 剩余金额

        uint64_t primary_key() const { return id; }
        uint64_t by_buyer() const { return buyer.value; } // 二级索引
    };

    // 修改 sales 表的定义，添加二级索引
    typedef eosio::multi_index<
        "sales"_n, sale,
        indexed_by<"bybuyer"_n, const_mem_fun<sale, uint64_t, &sale::by_buyer>>
    > sales_table;

    typedef eosio::multi_index<"accounts"_n, account> accounts_table;
    typedef eosio::multi_index<"soldaccounts"_n, sold_account> sold_accounts_table;

    sold_accounts_table sold_accounts;

    checksum256 get_trx_id() {
        auto size = transaction_size();
        char buf[size];
        uint32_t read = read_transaction(buf, size);
        check(size == read, "read_transaction failed");
        return sha256(buf, read);
    }

    const char* base58_chars = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
    std::vector<uint8_t> base58_to_binary(const std::string& input) {
        std::vector<uint8_t> result;
        for (char c : input) {
            const char* p = strchr(base58_chars, c);
            check(p != nullptr, "Invalid Base58 character.");
            int value = p - base58_chars;

            std::vector<uint8_t> temp_result;
            int carry = value;
            for (auto it = result.rbegin(); it != result.rend(); ++it) {
                int temp = *it * 58 + carry;
                temp_result.push_back(temp % 256);
                carry = temp / 256;
            }
            while (carry > 0) {
                temp_result.push_back(carry % 256);
                carry /= 256;
            }

            result.clear();
            result.reserve(temp_result.size());
            for (auto it = temp_result.rbegin(); it != temp_result.rend(); ++it) {
                result.push_back(*it);
            }
        }

        // Add leading zeros
        int numZeros = 0;
        for (numZeros = 0; numZeros < input.length(); ++numZeros) {
            if (input[numZeros] != base58_chars[0]) {
                break;
            }
        }
        std::vector<uint8_t> zeros(numZeros, 0);
        result.insert(result.begin(), zeros.begin(), zeros.end());

        // Remove the last 4 bytes (checksum) and return the payload
        check(result.size() >= 4, "Decoded data is too short.");
        result.resize(result.size() - 4);

        return result;
    }

    public_key is_valid_pubkey(const std::string& pubkey_str) {
        // 检查公钥是否以"EOS"开头
        if (pubkey_str.substr(0, 3) != "EOS") {
            eosio::check(false, "Invalid public key: does not start with 'EOS'");
        }

        // 解析并处理以"EOS"开头的公钥
        auto pubkey_data = base58_to_binary(pubkey_str.substr(3));

        // 创建并返回eosio::public_key对象
        eosio::ecc_public_key ecc_key;
        std::copy(pubkey_data.begin(), pubkey_data.end(), ecc_key.begin());
        return eosio::public_key{std::in_place_index<0>, ecc_key};
    }

    uint64_t sell_eos_for_ram(name account, asset quantity) {
        double price = get_ram_price();
        uint64_t bytes = quantity.amount * 0.995 / price;
        print("购买RAM数量 -> ", bytes);

        // 卖出 EOS 换取 RAM
        action(
            permission_level{account, "active"_n},
            "eosio"_n, 
            "buyram"_n,
            std::make_tuple(account, account, quantity)
        ).send();

        // 返回获得的 RAM 数量
        return bytes;
    }

    struct exchange_state {
        asset    supply;

        struct connector {

            asset balance;
            double weight = .5;
        };

        connector base;
        connector quote;

        uint64_t primary_key()const { return supply.symbol.raw(); }
    };

    typedef eosio::multi_index<"rammarket"_n, exchange_state> rammarket;

    double get_ram_price() {

        rammarket market("eosio"_n, "eosio"_n.value);
        auto itr = market.find(symbol("RAMCORE", 4).raw());
        eosio::check(itr != market.end(), "RAM market not found");

        return (double)itr->quote.balance.amount / (double)itr->base.balance.amount;
    }
};
