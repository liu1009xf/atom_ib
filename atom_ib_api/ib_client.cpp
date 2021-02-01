#include "ib_client.h"

#include <iostream>
#include <chrono>

namespace atom::ib{

    IBClient::IBClient():
        signal_(2),
        client_(EClientSocket(this, &signal_)),
        state_(STATE::BUSY)
    {

    }

    IBClient::IBClient(int sig_timeout):
        signal_(sig_timeout),
        client_(EClientSocket(this, &signal_)),
        state_(STATE::BUSY)
    {

    }

    //! [managedaccounts]
    void IBClient::managedAccounts( const std::string& accountsList) {
        printf( "Account List: %s\n", accountsList.c_str());
    }
    //! [managedaccounts]

    bool IBClient::connect(const char * host, int port, int clientId)
    {
        // trying to connect
        printf( "Connecting to %s:%d clientId:%d\n", !( host && *host) ? "127.0.0.1" : host, port, clientId);
        
        unsigned maxAtempt = 5;
        bool bRes = false;
        while(!isConnected()){
            //! [connect]
            std::future<bool> bRes_fut = std::async(std::launch::async, 
                [this, &host, &port, &clientId] { return this->client_.eConnect(host,port, clientId); }
            );
            //! [connect]
            
            unsigned count = 0;
            while (bRes_fut.wait_for(std::chrono::microseconds(100)) != std::future_status::ready) {
                printf( "Cannot connect to %s:%d clientId:%d\n", client_.host().c_str(), client_.port(), clientId);
                std::cout<<"Retry in 1 second!"<<std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                if(count > maxAtempt){
                    break;
                }
                ++count;
            }
            
            std::cout<<"Connected to "
                << client_.host().c_str()<<":"
                << client_.port()
                <<"clientId: "                
                << clientId
                <<std::endl;

            //! [ereader]
            bRes = bRes_fut.get();
            if(bRes){
                std::cout<<"Reader Created!"<<std::endl;
                try{
                    reader_ = std::unique_ptr<EReader>(new EReader(&client_, &signal_));
                    reader_->start();   
                }catch(...){
                    std::cout<<"failed to create reader"<<std::endl;
                }
            }
            std::cout<<"reader created"<<std::endl;

            std::this_thread::sleep_for(std::chrono::seconds(1));
            processMsg();
            //! [ereader]
        }
        return bRes;
    }

    void IBClient::currentTime( long time)
{
    time_t t = ( time_t)time;
    struct tm * timeinfo = localtime ( &t);
    printf( "The current date/time is: %s", asctime( timeinfo));
}
    void IBClient::connectAck() {
        if(!client_.asyncEConnect())
        {
            client_.startApi();
        }
    }
    void IBClient::nextValidId( OrderId orderId)
    {
        printf("Next Valid Id: %ld\n", orderId);
        state_ = STATE::FREE;

    }

    //! [tickoptioncomputation]
    void IBClient::tickOptionComputation( TickerId tickerId, TickType tickType, int tickAttrib, double impliedVol, double delta,
                                            double optPrice, double pvDividend,
                                            double gamma, double vega, double theta, double undPrice) {
        printf( "TickOptionComputation. Ticker Id: %ld, Type: %d, TickAttrib: %d, ImpliedVolatility: %g, Delta: %g, OptionPrice: %g, pvDividend: %g, Gamma: %g, Vega: %g, Theta: %g, Underlying Price: %g\n", tickerId, (int)tickType, tickAttrib, impliedVol, delta, optPrice, pvDividend, gamma, vega, theta, undPrice);
    }
    //! [tickoptioncomputation]


    void IBClient::replaceFAEnd(int reqId, const std::string& text) {
        printf("Replace FA End. Request: %d, Text:%s\n", reqId, text.c_str());
    }

    void IBClient::disconnect()
    {
        client_.eDisconnect();

        printf ( "Disconnected\n");
    }

    bool IBClient::isConnected()
    {
        return client_.isConnected();
    }

    void IBClient::error(int id, int errorCode, const std::string& errorString)
    {
        printf( "Error. Id: %d, Code: %d, Msg: %s\n", id, errorCode, errorString.c_str());
    }

    void IBClient::historicalData(TickerId reqId, const Bar& bar)
    {
        bar_.push_back(bar);
        printf( "HistoricalData. ReqId: %ld - Date: %s, Open: %g, High: %g, Low: %g, Close: %g, Volume: %lld, Count: %d, WAP: %g\n", reqId, bar.time.c_str(), bar.open, bar.high, bar.low, bar.close, bar.volume, bar.count, bar.wap);
    }

    void IBClient::historicalDataEnd(int reqId, const std::string& startDateStr, const std::string& endDateStr) {
	std::cout << "HistoricalDataEnd. ReqId: " << reqId << " - Start Date: " << startDateStr << ", End Date: " << endDateStr << std::endl;	
}

    //! [tickprice]
    void IBClient::tickPrice( TickerId tickerId, TickType field, double price, const TickAttrib& attribs) {
        printf( "Tick Price. Ticker Id: %ld, Field: %d, Price: %g, CanAutoExecute: %d, PastLimit: %d, PreOpen: %d\n", tickerId, (int)field, price, attribs.canAutoExecute, attribs.pastLimit, attribs.preOpen);
    }
    //! [tickprice]

    //! [ticksize]
    void IBClient::tickSize( TickerId tickerId, TickType field, int size) {
        printf( "Tick Size. Ticker Id: %ld, Field: %d, Size: %d\n", tickerId, (int)field, size);
    }
    //! [ticksize]
    
    //! [tickgeneric]
    void IBClient::tickGeneric(TickerId tickerId, TickType tickType, double value) {
        printf( "Tick Generic. Ticker Id: %ld, Type: %d, Value: %g\n", tickerId, (int)tickType, value);
    }
    //! [tickgeneric]

    //! [tickstring]
    void IBClient::tickString(TickerId tickerId, TickType tickType, const std::string& value) {
        printf( "Tick String. Ticker Id: %ld, Type: %d, Value: %s\n", tickerId, (int)tickType, value.c_str());
    }
    //! [tickstring]

    void IBClient::tickEFP(TickerId tickerId, TickType tickType, double basisPoints, const std::string& formattedBasisPoints,
                                double totalDividends, int holdDays, const std::string& futureLastTradeDate, double dividendImpact, double dividendsToLastTradeDate) {
        printf( "TickEFP. %ld, Type: %d, BasisPoints: %g, FormattedBasisPoints: %s, Total Dividends: %g, HoldDays: %d, Future Last Trade Date: %s, Dividend Impact: %g, Dividends To Last Trade Date: %g\n", tickerId, (int)tickType, basisPoints, formattedBasisPoints.c_str(), totalDividends, holdDays, futureLastTradeDate.c_str(), dividendImpact, dividendsToLastTradeDate);
    }

    //! [orderstatus]
    void IBClient::orderStatus(OrderId orderId, const std::string& status, double filled,
            double remaining, double avgFillPrice, int permId, int parentId,
            double lastFillPrice, int clientId, const std::string& whyHeld, double mktCapPrice){
        printf("OrderStatus. Id: %ld, Status: %s, Filled: %g, Remaining: %g, AvgFillPrice: %g, PermId: %d, LastFillPrice: %g, ClientId: %d, WhyHeld: %s, MktCapPrice: %g\n", orderId, status.c_str(), filled, remaining, avgFillPrice, permId, lastFillPrice, clientId, whyHeld.c_str(), mktCapPrice);
    }
    //! [orderstatus]

    //! [openorder]
    void IBClient::openOrder( OrderId orderId, const Contract& contract, const Order& order, const OrderState& orderState) {
    }
    //! [openorder]

    //! [openorderend]
    void IBClient::openOrderEnd() {
        printf( "OpenOrderEnd\n");
    }
    //! [openorderend]

    void IBClient::winError( const std::string& str, int lastError) {}
    void IBClient::connectionClosed() {
        printf( "Connection Closed\n");
    }

    //! [updateaccountvalue]
    void IBClient::updateAccountValue(const std::string& key, const std::string& val,
                                        const std::string& currency, const std::string& accountName) {
        printf("UpdateAccountValue. Key: %s, Value: %s, Currency: %s, Account Name: %s\n", key.c_str(), val.c_str(), currency.c_str(), accountName.c_str());
    }
    //! [updateaccountvalue]

    //! [updateportfolio]
    void IBClient::updatePortfolio(const Contract& contract, double position,
                                        double marketPrice, double marketValue, double averageCost,
                                        double unrealizedPNL, double realizedPNL, const std::string& accountName){
        printf("UpdatePortfolio. %s, %s @ %s: Position: %g, MarketPrice: %g, MarketValue: %g, AverageCost: %g, UnrealizedPNL: %g, RealizedPNL: %g, AccountName: %s\n", (contract.symbol).c_str(), (contract.secType).c_str(), (contract.primaryExchange).c_str(), position, marketPrice, marketValue, averageCost, unrealizedPNL, realizedPNL, accountName.c_str());
    }
    //! [updateportfolio]

    //! [updateaccounttime]
    void IBClient::updateAccountTime(const std::string& timeStamp) {
        printf( "UpdateAccountTime. Time: %s\n", timeStamp.c_str());
    }
    //! [updateaccounttime]

    //! [accountdownloadend]
    void IBClient::accountDownloadEnd(const std::string& accountName) {
        printf( "Account download finished: %s\n", accountName.c_str());
    }
    //! [accountdownloadend]

    //! [contractdetails]
    void IBClient::contractDetails( int reqId, const ContractDetails& contractDetails) {
        printf( "ContractDetails begin. ReqId: %d\n", reqId);
        printf( "ContractDetails end. ReqId: %d\n", reqId);
    }
    //! [contractdetails]

    //! [bondcontractdetails]
    void IBClient::bondContractDetails( int reqId, const ContractDetails& contractDetails) {
        printf( "BondContractDetails begin. ReqId: %d\n", reqId);
        printf( "BondContractDetails end. ReqId: %d\n", reqId);
    }
    //! [bondcontractdetails]

    //! [contractdetailsend]
    void IBClient::contractDetailsEnd( int reqId) {
        printf( "ContractDetailsEnd. %d\n", reqId);
    }
    //! [contractdetailsend]

    //! [execdetails]
    void IBClient::execDetails( int reqId, const Contract& contract, const Execution& execution) {
    }
    //! [execdetails]

    //! [execdetailsend]
    void IBClient::execDetailsEnd( int reqId) {
        printf( "ExecDetailsEnd. %d\n", reqId);
    }
    //! [execdetailsend]

    //! [updatemktdepth]
    void IBClient::updateMktDepth(TickerId id, int position, int operation, int side,
                                    double price, int size) {
        printf( "UpdateMarketDepth. %ld - Position: %d, Operation: %d, Side: %d, Price: %g, Size: %d\n", id, position, operation, side, price, size);
    }
    //! [updatemktdepth]

    //! [updatemktdepthl2]
    void IBClient::updateMktDepthL2(TickerId id, int position, const std::string& marketMaker, int operation,
                                        int side, double price, int size, bool isSmartDepth) {
        printf( "UpdateMarketDepthL2. %ld - Position: %d, Operation: %d, Side: %d, Price: %g, Size: %d, isSmartDepth: %d\n", id, position, operation, side, price, size, isSmartDepth);
    }
    //! [updatemktdepthl2]

    //! [updatenewsbulletin]
    void IBClient::updateNewsBulletin(int msgId, int msgType, const std::string& newsMessage, const std::string& originExch) {
        printf( "News Bulletins. %d - Type: %d, Message: %s, Exchange of Origin: %s\n", msgId, msgType, newsMessage.c_str(), originExch.c_str());
    }
    //! [updatenewsbulletin]

   

    //! [receivefa]
    void IBClient::receiveFA(faDataType pFaDataType, const std::string& cxml) {
        std::cout << "Receiving FA: " << (int)pFaDataType << std::endl << cxml << std::endl;
    }
    //! [receivefa]

    //! [scannerparameters]
    void IBClient::scannerParameters(const std::string& xml) {
        printf( "ScannerParameters. %s\n", xml.c_str());
    }
    //! [scannerparameters]

    //! [scannerdata]
    void IBClient::scannerData(int reqId, int rank, const ContractDetails& contractDetails,
                                    const std::string& distance, const std::string& benchmark, const std::string& projection,
                                    const std::string& legsStr) {
        printf( "ScannerData. %d - Rank: %d, Symbol: %s, SecType: %s, Currency: %s, Distance: %s, Benchmark: %s, Projection: %s, Legs String: %s\n", reqId, rank, contractDetails.contract.symbol.c_str(), contractDetails.contract.secType.c_str(), contractDetails.contract.currency.c_str(), distance.c_str(), benchmark.c_str(), projection.c_str(), legsStr.c_str());
    }
    //! [scannerdata]

    //! [scannerdataend]
    void IBClient::scannerDataEnd(int reqId) {
        printf( "ScannerDataEnd. %d\n", reqId);
    }
    //! [scannerdataend]

    //! [realtimebar]
    void IBClient::realtimeBar(TickerId reqId, long time, double open, double high, double low, double close,
                                    long volume, double wap, int count) {
        printf( "RealTimeBars. %ld - Time: %ld, Open: %g, High: %g, Low: %g, Close: %g, Volume: %ld, Count: %d, WAP: %g\n", reqId, time, open, high, low, close, volume, count, wap);
    }
    //! [realtimebar]

    //! [fundamentaldata]
    void IBClient::fundamentalData(TickerId reqId, const std::string& data) {
        printf( "FundamentalData. ReqId: %ld, %s\n", reqId, data.c_str());
    }
    //! [fundamentaldata]

    void IBClient::deltaNeutralValidation(int reqId, const DeltaNeutralContract& deltaNeutralContract) {
        printf( "DeltaNeutralValidation. %d, ConId: %ld, Delta: %g, Price: %g\n", reqId, deltaNeutralContract.conId, deltaNeutralContract.delta, deltaNeutralContract.price);
    }

    //! [ticksnapshotend]
    void IBClient::tickSnapshotEnd(int reqId) {
        printf( "TickSnapshotEnd: %d\n", reqId);
    }
    //! [ticksnapshotend]

    //! [marketdatatype]
    void IBClient::marketDataType(TickerId reqId, int marketDataType) {
        printf( "MarketDataType. ReqId: %ld, Type: %d\n", reqId, marketDataType);
    }
    //! [marketdatatype]

    //! [commissionreport]
    void IBClient::commissionReport( const CommissionReport& commissionReport) {
    }
    //! [commissionreport]

    //! [position]
    void IBClient::position( const std::string& account, const Contract& contract, double position, double avgCost) {
        printf( "Position. %s - Symbol: %s, SecType: %s, Currency: %s, Position: %g, Avg Cost: %g\n", account.c_str(), contract.symbol.c_str(), contract.secType.c_str(), contract.currency.c_str(), position, avgCost);
    }
    //! [position]

    //! [positionend]
    void IBClient::positionEnd() {
        printf( "PositionEnd\n");
    }
    //! [positionend]

    //! [accountsummary]
    void IBClient::accountSummary( int reqId, const std::string& account, const std::string& tag, const std::string& value, const std::string& currency) {
        printf( "Acct Summary. ReqId: %d, Account: %s, Tag: %s, Value: %s, Currency: %s\n", reqId, account.c_str(), tag.c_str(), value.c_str(), currency.c_str());
    }
    //! [accountsummary]

    //! [accountsummaryend]
    void IBClient::accountSummaryEnd( int reqId) {
        printf( "AccountSummaryEnd. Req Id: %d\n", reqId);
    }
    //! [accountsummaryend]

    void IBClient::verifyMessageAPI( const std::string& apiData) {
        printf("verifyMessageAPI: %s\b", apiData.c_str());
    }

    void IBClient::verifyCompleted( bool isSuccessful, const std::string& errorText) {
        printf("verifyCompleted. IsSuccessfule: %d - Error: %s\n", isSuccessful, errorText.c_str());
    }

    void IBClient::verifyAndAuthMessageAPI( const std::string& apiDatai, const std::string& xyzChallenge) {
        printf("verifyAndAuthMessageAPI: %s %s\n", apiDatai.c_str(), xyzChallenge.c_str());
    }

    void IBClient::verifyAndAuthCompleted( bool isSuccessful, const std::string& errorText) {
        printf("verifyAndAuthCompleted. IsSuccessful: %d - Error: %s\n", isSuccessful, errorText.c_str());
        if (isSuccessful)
            client_.startApi();
    }

    //! [displaygrouplist]
    void IBClient::displayGroupList( int reqId, const std::string& groups) {
        printf("Display Group List. ReqId: %d, Groups: %s\n", reqId, groups.c_str());
    }
    //! [displaygrouplist]

    //! [displaygroupupdated]
    void IBClient::displayGroupUpdated( int reqId, const std::string& contractInfo) {
        std::cout << "Display Group Updated. ReqId: " << reqId << ", Contract Info: " << contractInfo << std::endl;
    }
    //! [displaygroupupdated]

    //! [positionmulti]
    void IBClient::positionMulti( int reqId, const std::string& account,const std::string& modelCode, const Contract& contract, double pos, double avgCost) {
        printf("Position Multi. Request: %d, Account: %s, ModelCode: %s, Symbol: %s, SecType: %s, Currency: %s, Position: %g, Avg Cost: %g\n", reqId, account.c_str(), modelCode.c_str(), contract.symbol.c_str(), contract.secType.c_str(), contract.currency.c_str(), pos, avgCost);
    }
    //! [positionmulti]

    //! [positionmultiend]
    void IBClient::positionMultiEnd( int reqId) {
        printf("Position Multi End. Request: %d\n", reqId);
    }
    //! [positionmultiend]

    //! [accountupdatemulti]
    void IBClient::accountUpdateMulti( int reqId, const std::string& account, const std::string& modelCode, const std::string& key, const std::string& value, const std::string& currency) {
        printf("AccountUpdate Multi. Request: %d, Account: %s, ModelCode: %s, Key, %s, Value: %s, Currency: %s\n", reqId, account.c_str(), modelCode.c_str(), key.c_str(), value.c_str(), currency.c_str());
    }
    //! [accountupdatemulti]

    //! [accountupdatemultiend]
    void IBClient::accountUpdateMultiEnd( int reqId) {
        printf("Account Update Multi End. Request: %d\n", reqId);
    }
    //! [accountupdatemultiend]

    //! [securityDefinitionOptionParameter]
    void IBClient::securityDefinitionOptionalParameter(int reqId, const std::string& exchange, int underlyingConId, const std::string& tradingClass,
                                                            const std::string& multiplier, const std::set<std::string>& expirations, const std::set<double>& strikes) {
        printf("Security Definition Optional Parameter. Request: %d, Trading Class: %s, Multiplier: %s\n", reqId, tradingClass.c_str(), multiplier.c_str());
    }
    //! [securityDefinitionOptionParameter]

    //! [securityDefinitionOptionParameterEnd]
    void IBClient::securityDefinitionOptionalParameterEnd(int reqId) {
        printf("Security Definition Optional Parameter End. Request: %d\n", reqId);
    }
    //! [securityDefinitionOptionParameterEnd]

    //! [softDollarTiers]
    void IBClient::softDollarTiers(int reqId, const std::vector<SoftDollarTier> &tiers) {
        printf("Soft dollar tiers (%lu):", tiers.size());

        for (unsigned int i = 0; i < tiers.size(); i++) {
            printf("%s\n", tiers[i].displayName().c_str());
        }
    }
    //! [softDollarTiers]

    //! [familyCodes]
    void IBClient::familyCodes(const std::vector<FamilyCode> &familyCodes) {
        printf("Family codes (%lu):\n", familyCodes.size());

        for (unsigned int i = 0; i < familyCodes.size(); i++) {
            printf("Family code [%d] - accountID: %s familyCodeStr: %s\n", i, familyCodes[i].accountID.c_str(), familyCodes[i].familyCodeStr.c_str());
        }
    }
    //! [familyCodes]

    //! [symbolSamples]
    void IBClient::symbolSamples(int reqId, const std::vector<ContractDescription> &contractDescriptions) {
        printf("Symbol Samples (total=%lu) reqId: %d\n", contractDescriptions.size(), reqId);

        for (unsigned int i = 0; i < contractDescriptions.size(); i++) {
            Contract contract = contractDescriptions[i].contract;
            std::vector<std::string> derivativeSecTypes = contractDescriptions[i].derivativeSecTypes;
            printf("Contract (%u): %ld %s %s %s %s, ", i, contract.conId, contract.symbol.c_str(), contract.secType.c_str(), contract.primaryExchange.c_str(), contract.currency.c_str());
            printf("Derivative Sec-types (%lu):", derivativeSecTypes.size());
            for (unsigned int j = 0; j < derivativeSecTypes.size(); j++) {
                printf(" %s", derivativeSecTypes[j].c_str());
            }
            printf("\n");
        }
    }
    //! [symbolSamples]

    //! [mktDepthExchanges]
    void IBClient::mktDepthExchanges(const std::vector<DepthMktDataDescription> &depthMktDataDescriptions) {
        printf("Mkt Depth Exchanges (%lu):\n", depthMktDataDescriptions.size());

        for (unsigned int i = 0; i < depthMktDataDescriptions.size(); i++) {
            printf("Depth Mkt Data Description [%d] - exchange: %s secType: %s listingExch: %s serviceDataType: %s aggGroup: %s\n", i, 
                depthMktDataDescriptions[i].exchange.c_str(), 
                depthMktDataDescriptions[i].secType.c_str(), 
                depthMktDataDescriptions[i].listingExch.c_str(), 
                depthMktDataDescriptions[i].serviceDataType.c_str(), 
                depthMktDataDescriptions[i].aggGroup != INT_MAX ? std::to_string(depthMktDataDescriptions[i].aggGroup).c_str() : "");
        }
    }
    //! [mktDepthExchanges]

    //! [tickNews]
    void IBClient::tickNews(int tickerId, time_t timeStamp, const std::string& providerCode, const std::string& articleId, const std::string& headline, const std::string& extraData) {
        printf("News Tick. TickerId: %d, TimeStamp: %s, ProviderCode: %s, ArticleId: %s, Headline: %s, ExtraData: %s\n", tickerId, ctime(&(timeStamp /= 1000)), providerCode.c_str(), articleId.c_str(), headline.c_str(), extraData.c_str());
    }
    //! [tickNews]

    //! [smartcomponents]]
    void IBClient::smartComponents(int reqId, const SmartComponentsMap& theMap) {
        printf("Smart components: (%lu):\n", theMap.size());

        for (SmartComponentsMap::const_iterator i = theMap.begin(); i != theMap.end(); i++) {
            printf(" bit number: %d exchange: %s exchange letter: %c\n", i->first, std::get<0>(i->second).c_str(), std::get<1>(i->second));
        }
    }
    //! [smartcomponents]

    //! [tickReqParams]
    void IBClient::tickReqParams(int tickerId, double minTick, const std::string& bboExchange, int snapshotPermissions) {
    }
    //! [tickReqParams]

    //! [newsProviders]
    void IBClient::newsProviders(const std::vector<NewsProvider> &newsProviders) {
        printf("News providers (%lu):\n", newsProviders.size());

        for (unsigned int i = 0; i < newsProviders.size(); i++) {
            printf("News provider [%d] - providerCode: %s providerName: %s\n", i, newsProviders[i].providerCode.c_str(), newsProviders[i].providerName.c_str());
        }
    }
    //! [newsProviders]

    //! [newsArticle]
    void IBClient::newsArticle(int requestId, int articleType, const std::string& articleText) {
        printf("News Article. Request Id: %d, Article Type: %d\n", requestId, articleType);
    }
    //! [newsArticle]

    //! [historicalNews]
    void IBClient::historicalNews(int requestId, const std::string& time, const std::string& providerCode, const std::string& articleId, const std::string& headline) {
        printf("Historical News. RequestId: %d, Time: %s, ProviderCode: %s, ArticleId: %s, Headline: %s\n", requestId, time.c_str(), providerCode.c_str(), articleId.c_str(), headline.c_str());
    }
    //! [historicalNews]

    //! [historicalNewsEnd]
    void IBClient::historicalNewsEnd(int requestId, bool hasMore) {
        printf("Historical News End. RequestId: %d, HasMore: %s\n", requestId, (hasMore ? "true" : " false"));
    }
    //! [historicalNewsEnd]

    //! [headTimestamp]
    void IBClient::headTimestamp(int reqId, const std::string& headTimestamp) {
        printf( "Head time stamp. ReqId: %d - Head time stamp: %s,\n", reqId, headTimestamp.c_str());

    }
    //! [headTimestamp]

    //! [histogramData]
    void IBClient::histogramData(int reqId, const HistogramDataVector& data) {
        printf("Histogram. ReqId: %d, data length: %lu\n", reqId, data.size());

        for (auto item : data) {
            printf("\t price: %f, size: %lld\n", item.price, item.size);
        }
    }
    //! [histogramData]

    //! [historicalDataUpdate]
    void IBClient::historicalDataUpdate(TickerId reqId, const Bar& bar) {
        printf( "HistoricalDataUpdate. ReqId: %ld - Date: %s, Open: %g, High: %g, Low: %g, Close: %g, Volume: %lld, Count: %d, WAP: %g\n", reqId, bar.time.c_str(), bar.open, bar.high, bar.low, bar.close, bar.volume, bar.count, bar.wap);
    }
    //! [historicalDataUpdate]

    //! [rerouteMktDataReq]
    void IBClient::rerouteMktDataReq(int reqId, int conid, const std::string& exchange) {
        printf( "Re-route market data request. ReqId: %d, ConId: %d, Exchange: %s\n", reqId, conid, exchange.c_str());
    }
    //! [rerouteMktDataReq]

    //! [rerouteMktDepthReq]
    void IBClient::rerouteMktDepthReq(int reqId, int conid, const std::string& exchange) {
        printf( "Re-route market depth request. ReqId: %d, ConId: %d, Exchange: %s\n", reqId, conid, exchange.c_str());
    }
    //! [rerouteMktDepthReq]

    //! [marketRule]
    void IBClient::marketRule(int marketRuleId, const std::vector<PriceIncrement> &priceIncrements) {
        printf("Market Rule Id: %d\n", marketRuleId);
        for (unsigned int i = 0; i < priceIncrements.size(); i++) {
            printf("Low Edge: %g, Increment: %g\n", priceIncrements[i].lowEdge, priceIncrements[i].increment);
        }
    }
    //! [marketRule]

    //! [pnl]
    void IBClient::pnl(int reqId, double dailyPnL, double unrealizedPnL, double realizedPnL) {
        printf("PnL. ReqId: %d, daily PnL: %g, unrealized PnL: %g, realized PnL: %g\n", reqId, dailyPnL, unrealizedPnL, realizedPnL);
    }
    //! [pnl]

    //! [pnlsingle]
    void IBClient::pnlSingle(int reqId, int pos, double dailyPnL, double unrealizedPnL, double realizedPnL, double value) {
        printf("PnL Single. ReqId: %d, pos: %d, daily PnL: %g, unrealized PnL: %g, realized PnL: %g, value: %g\n", reqId, pos, dailyPnL, unrealizedPnL, realizedPnL, value);
    }
    //! [pnlsingle]

    //! [historicalticks]
    void IBClient::historicalTicks(int reqId, const std::vector<HistoricalTick>& ticks, bool done) {
        for (HistoricalTick tick : ticks) {
        std::time_t t = tick.time;
            std::cout << "Historical tick. ReqId: " << reqId << ", time: " << ctime(&t) << ", price: "<< tick.price << ", size: " << tick.size << std::endl;
        }
    }
    //! [historicalticks]

    //! [historicalticksbidask]
    void IBClient::historicalTicksBidAsk(int reqId, const std::vector<HistoricalTickBidAsk>& ticks, bool done) {
        for (HistoricalTickBidAsk tick : ticks) {
        std::time_t t = tick.time;
            std::cout << "Historical tick bid/ask. ReqId: " << reqId << ", time: " << ctime(&t) << ", price bid: "<< tick.priceBid <<
                ", price ask: "<< tick.priceAsk << ", size bid: " << tick.sizeBid << ", size ask: " << tick.sizeAsk <<
                ", bidPastLow: " << tick.tickAttribBidAsk.bidPastLow << ", askPastHigh: " << tick.tickAttribBidAsk.askPastHigh << std::endl;
        }
    }
    //! [historicalticksbidask]

    //! [historicaltickslast]
    void IBClient::historicalTicksLast(int reqId, const std::vector<HistoricalTickLast>& ticks, bool done) {
        for (HistoricalTickLast tick : ticks) {
        std::time_t t = tick.time;
            std::cout << "Historical tick last. ReqId: " << reqId << ", time: " << ctime(&t) << ", price: "<< tick.price <<
                ", size: " << tick.size << ", exchange: " << tick.exchange << ", special conditions: " << tick.specialConditions <<
                ", unreported: " << tick.tickAttribLast.unreported << ", pastLimit: " << tick.tickAttribLast.pastLimit << std::endl;
        }
    }
    //! [historicaltickslast]

    //! [tickbytickalllast]
    void IBClient::tickByTickAllLast(int reqId, int tickType, time_t time, double price, int size, const TickAttribLast& tickAttribLast, const std::string& exchange, const std::string& specialConditions) {
        printf("Tick-By-Tick. ReqId: %d, TickType: %s, Time: %s, Price: %g, Size: %d, PastLimit: %d, Unreported: %d, Exchange: %s, SpecialConditions:%s\n", 
            reqId, (tickType == 1 ? "Last" : "AllLast"), ctime(&time), price, size, tickAttribLast.pastLimit, tickAttribLast.unreported, exchange.c_str(), specialConditions.c_str());
    }
    //! [tickbytickalllast]

    //! [tickbytickbidask]
    void IBClient::tickByTickBidAsk(int reqId, time_t time, double bidPrice, double askPrice, int bidSize, int askSize, const TickAttribBidAsk& tickAttribBidAsk) {
        printf("Tick-By-Tick. ReqId: %d, TickType: BidAsk, Time: %s, BidPrice: %g, AskPrice: %g, BidSize: %d, AskSize: %d, BidPastLow: %d, AskPastHigh: %d\n", 
            reqId, ctime(&time), bidPrice, askPrice, bidSize, askSize, tickAttribBidAsk.bidPastLow, tickAttribBidAsk.askPastHigh);
    }
    //! [tickbytickbidask]

    //! [tickbytickmidpoint]
    void IBClient::tickByTickMidPoint(int reqId, time_t time, double midPoint) {
        printf("Tick-By-Tick. ReqId: %d, TickType: MidPoint, Time: %s, MidPoint: %g\n", reqId, ctime(&time), midPoint);
    }
    //! [tickbytickmidpoint]

    //! [orderbound]
    void IBClient::orderBound(long long orderId, int apiClientId, int apiOrderId) {
        printf("Order bound. OrderId: %lld, ApiClientId: %d, ApiOrderId: %d\n", orderId, apiClientId, apiOrderId);
    }
    //! [orderbound]

    //! [completedorder]
    void IBClient::completedOrder(const Contract& contract, const Order& order, const OrderState& orderState) {}
    //! [completedorder]

    //! [completedordersend]
    void IBClient::completedOrdersEnd() {
        printf( "CompletedOrdersEnd\n");
    }

}