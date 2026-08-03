#include "Utils.h"
#include <QDir>
#include <QTime>
#include <QSettings>
#include <QFile>
#include <QCryptographicHash>

#include <QEventLoop>
#include <QFile>
#include <QTimer>
#include <QTextCodec>
#include <QSet>
#include <QProcess>
#include <QBuffer>
#include <QTemporaryFile>
#include <bitset>

#ifdef Q_OS_WIN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#include <comdef.h>
#include <Wbemidl.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "Advapi32.lib")

#elif defined(Q_OS_LINUX)
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif

#ifdef HAVE_TCMALLOC
#include <google/heap-profiler.h>
#include <google/tcmalloc.h>
#endif

#ifdef HAVE_TCMALLOC
#include <google/heap-profiler.h>
#include <google/tcmalloc.h>
#include <google/malloc_extension.h>
#endif

#ifdef QT_NETWORK_LIB
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkInterface>
#include <QHostAddress>
#endif

// #include <malloc.h>

QByteArray Utils::jsonToString(const QVariantMap &value, bool base64)
{
    return jsonToString(QJsonObject::fromVariantMap(value), base64);
}

QByteArray Utils::jsonToString(const QVariantList &value, bool base64)
{
    return jsonToString(QJsonArray::fromVariantList(value), base64);
}

QByteArray Utils::jsonToString(const QJsonValue &value, bool base64)
{
    QJsonDocument doc;

    switch (value.type()) {
    case QJsonValue::Object:
        doc.setObject(value.toObject());
        break;
    case QJsonValue::Array:
        doc.setArray(value.toArray());
        break;
    default:
        break;
    }

    auto data = doc.toJson(/*QJsonDocument::Compact*/);
    if (base64)
        data = data.toBase64();
    return data;
}

QJsonValue Utils::stringToJson(const QByteArray &data, bool base64)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(base64 ? QByteArray::fromBase64(data) :
                                                         data,
                                                &error);

    if (!data.isEmpty() && error.error != QJsonParseError::NoError)
        LOGW("%s: %s", qUtf8Printable(error.errorString()), data.data());

    if (doc.isObject())
        return doc.object();
    else if (doc.isArray())
        return doc.array();
    else
        return QJsonValue();
}

qreal Utils::jsonValueToDouble(const QJsonValue &v)
{
    if (v.isDouble())
        return v.toDouble();
    else if(v.isString())
        return v.toString().toDouble();
    else
        return v.toInt();
}

QString Utils::jsonValueToString(const QJsonValue &v)
{
    if (v.isString())
        return v.toString();
    else if(v.isDouble())
        return QString::number(v.toDouble());
    else
        return QString::number(v.toInt());
}

QStringList Utils::arrayToStringList(const QJsonArray &array)
{
    QStringList list;
    for (const auto &item: array)
        list.append(item.toString());
    return list;
}

std::list<std::string> Utils::arrayToStdStringList(const QJsonArray &array)
{
    std::list<std::string> list;
    for (const auto &item: array)
        list.push_back(item.toString().toStdString());
    return list;
}

QJsonValue Utils::stringListToArray(const QStringList &list)
{
    QJsonArray array;
    for (auto &item: list) {
        array.append(item);
    }
    return array;
}

QJsonValue Utils::stringListToArray(const std::list<std::string> &list)
{
    QJsonArray array;
    for (auto &item: list) {
        array.append(item.c_str());
    }
    return array;
}

QVariantList Utils::stringListToVariantList(const QStringList &list)
{
    QVariantList array;
    for (auto &item: list) {
        array.append(item);
    }
    return array;
}

QStringList Utils::variantListToStringList(const QVariantList &list)
{
    QStringList array;
    for (auto &item: list) {
        array.append(item.toString());
    }
    return array;
}

bool Utils::writeToFile(const QString &filename, const QByteArray &data)
{
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(data);
        file.close();
        return true;
    }
    else {
        LOGW("open error: %s", qUtf8Printable(filename));
    }
    return false;
}

bool Utils::writeToFile(const QString &filename, const QVariantList &list)
{
    auto array = QJsonArray::fromVariantList(list);
    QJsonDocument doc(array);
    return writeToFile(filename, doc.toJson());
}

bool Utils::writeToFile(const QString &filename, const QStringList &list)
{
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        for (auto &line: list) {
            file.write(line.toUtf8());
            file.write("\n");
        }
        file.close();
        return true;
    }
    else {
        LOGW("open error: %s", qUtf8Printable(filename));
    }
    return false;
}

QByteArray Utils::readFromFile(const QString &filename)
{
    QByteArray data;
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly)) {
        data = file.readAll();
        file.close();
    }
    else {
        LOGW("open error: %s", qUtf8Printable(filename));
    }
    return data;
}

QJsonArray Utils::readFromFileToJsonArray(const QString &filename)
{
    QByteArray data = readFromFile(filename);
    if (data.isEmpty())
        return QJsonArray();

    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(data, &error);
    if (error.error == QJsonParseError::NoError) {
        return doc.array();
    }
    else {
        LOGW("%s: %s", qUtf8Printable(filename), qUtf8Printable(error.errorString()));
    }

    return QJsonArray();
}

void Utils::removeFile(QString path, const QStringList &nameFilters)
{
    QDir dir(path);
    auto infos = dir.entryInfoList(nameFilters);
    for (auto &info: infos) {
        dir.remove(info.fileName());
    }
}

bool Utils::removeFolder(QString path)
{
    QDir dir(path);
    return dir.removeRecursively();
}

bool Utils::clearFolder(QString path)
{
    bool res = true;

    QDir dir(path);
    auto infos = dir.entryInfoList(QDir::NoDotAndDotDot);
    for (auto &info: infos) {
        if (info.isDir())
            res = removeFolder(info.absoluteFilePath());
        else if (info.isFile())
            res = QFile::remove(info.absoluteFilePath());

        if (!res)
            break;
    }

    return res;
}

QString Utils::formatFileSize(qint64 size)
{
    qint64 KB = 1000LL;
    qint64 MB = 1000LL*1024;
    qint64 GB = 1000LL*1024*1024;
    qint64 TB = 1000LL*1024*1024*1024;

    if (size < KB)
        return QString::number(size) + " B";
    else if (size < MB)
        return QString::number((qreal)size / 1024, 'f', 2) + " KB";
    else if (size < GB)
        return QString::number((qreal)size / 1024 / 1024, 'f', 2) + " MB";
    else if (size < TB)
        return QString::number((qreal)size / 1024 / 1024 / 1024, 'f', 2) + " GB";
    else
        return QString::number((qreal)size / 1024 / 1024 / 1024 / 1024, 'f', 2) + " TB";
}

QString Utils::formatTimestamp(qint64 t, bool show_msec)
{
    qint64 day_msecs = 24LL*60*60*1000;
    qint64 hour_msecs = 60LL*60*1000;
    QString format = /*t < hour_msecs ? "mm:ss" :*/ "hh:mm:ss";
    if (show_msec)
        format.append(".zzz");
    QString str = QTime(0,0,0).addMSecs(t).toString(format);
    if (t >= day_msecs) {
        int d = t/day_msecs;
        str.prepend(QString("%1-").arg(d));
    }
    return str;
}

QString Utils::generateUuid()
{
    auto uuid = QUuid::createUuid().toString().toUpper();
    uuid = uuid.remove("{");
    uuid = uuid.remove("}");
    uuid = uuid.remove("-");
    return uuid;
}

QString Utils::getFirstLetter(QString str)
{
    static const char *wcsFiestLetterTable = ""
        "ydkqsxnwzssxjbymgcczqpssqbycdscdqldylybssjgyqzjjfgcclzznwdwzjljpfyynnjjtmynzwzhflzppqhgccyynmjqyxxgdnnsnsjnjnsnnmlnrxyfsngnnnnqzggllyjlnyzssecykyyhqwjssggyxyqyjtwktjhychmnxjtlhjyqbyxdldwrrjnwysrldzjpcbzjjbrcfslnczstzfxxchtrqggddlyccssymmrjcyqzpwwjjyfcrwfdfzqpyddwyxkyjawjffxjbcftzyhhycyswccyxsclcxxwzcxnbgnnxbxlzsqsbsjpysazdhmdzbqbscwdzzyytzhbtsyyfzgntnxjywqnknphhlxgybfmjnbjhhgqtjcysxstkzglyckglysmzxyalmeldccxgzyrjxjzlnjzcqkcnnjwhjczccqljststbnhbtyxceqxkkwjyflzqlyhjxspsfxlmpbysxxxytccnylllsjxfhjxpjbtffyabyxbcczbzyclwlczggbtssmdtjcxpthyqtgjjxcjfzkjzjqnlzwlslhdzbwjncjzyzsqnycqynzcjjwybrtwpyftwexcskdzctbyhyzqyyjxzcfbzzmjyxxsdczottbzljwfckscsxfyrlrygmbdthjxsqjccsbxyytswfbjdztnbcnzlcyzzpsacyzzsqqcshzqydxlbpjllmqxqydzxsqjtzpxlcglqdcwzfhctdjjsfxjejjtlbgxsxjmyjjqpfzasyjnsydjxkjcdjsznbartcclnjqmwnqnclllkbdbzzsyhqcltwlccrshllzntylnewyzyxczxxgdkdmtcedejtsyyssdqdfmxdbjlkrwnqlybglxnlgtgxbqjdznyjsjyjcjmrnymgrcjczgjmzmgxmmryxkjnymsgmzzymknfxmbdtgfbhcjhkylpfmdxlxjjsmsqgzsjlqdldgjycalcmzcsdjllnxdjffffjcnfnnffpfkhkgdpqxktacjdhh"
        "zdddrrcfqyjkqccwjdxhwjlyllzgcfcqjsmlzpbjjblsbcjggdckkdezsqcckjgcgkdjtjllzycxklqccgjcltfpcqczgwbjdqyzjjbyjhsjddwgfsjgzkcjctllfspkjgqjhzzljplgjgjjthjjyjzccmlzlyqbgjwmljkxzdznjqsyzmljlljkywxmkjlhskjhbmclyymkxjqlbmllkmdxxkwyxwslmlpsjqqjqxyqfjtjdxmxxllcrqbsyjbgwynnggbcnxpjtgpapfgdjqbhbncfjyzjkjkhxqfgqckfhygkhdkllsdjqxpqyaybnqsxqnszswhbsxwhxwbzzxdmndjbsbkbbzklylxgwxjjwaqzmywsjqlsjxxjqwjeqxnchetlzalyyyszzpnkyzcptlshtzcfycyxyljsdcjqagyslcllyyysslqqqnldxzsccscadycjysfsgbfrsszqsbxjpsjysdrckgjlgtkzjzbdktcsyqpyhstcldjnhmymcgxyzhjdctmhltxzhylamoxyjcltyfbqqjpfbdfehthsqhzywwcncxcdwhowgyjlegmdqcwgfjhcsntmydolbygnqwesqpwnmlrydzszzlyqpzgcwxhnxpyxshmdqjgztdppbfbhzhhjyfdzwkgkzbldnzsxhqeegzxylzmmzyjzgszxkhkhtxexxgylyapsthxdwhzydpxagkydxbhnhnkdnjnmyhylpmgecslnzhkxxlbzzlbmlsfbhhgsgyyggbhscyajtxglxtzmcwzydqdqmngdnllszhngjzwfyhqswscelqajynytlsxthaznkzzsdhlaxxtwwcjhqqtddwzbcchyqzflxpslzqgpzsznglydqtbdlxntctajdkywnsyzljhhdzckryyzywmhychhhxhjkzwsxhdnxlyscqydpslyzwmypnkxyjlkchtyhaxqsyshxasmchkdscrsgjpwqsgzjlwwschsjhsqnhnsngndantbaalczmss"
        "tdqjcjktscjnxplggxhhgoxzcxpdmmhldgtybynjmxhmrzplxjzckzxshflqxxcdhxwzpckczcdytcjyxqhlxdhypjqxnlsyydzozjnhhqezysjyayxkypdgxddnsppyzndhthrhxydpcjjhtcnnctlhbynyhmhzllnnxmylllmdcppxhmxdkycyrdltxjchhznxclcclylnzsxnjzzlnnnnwhyqsnjhxynttdkyjpychhyegkcwtwlgjrlggtgtygyhpyhylqyqgcwyqkpyyettttlhyylltyttsylnyzwgywgpydqqzzdqnnkcqnmjjzzbxtqfjkdffbtkhzkbxdjjkdjjtlbwfzpptkqtztgpdwntpjyfalqmkgxbcclzfhzcllllanpnxtjklcclgyhdzfgyddgcyyfgydxkssendhykdndknnaxxhbpbyyhxccgapfqyjjdmlxcsjzllpcnbsxgjyndybwjspcwjlzkzddtacsbkzdyzypjzqsjnkktknjdjgyepgtlnyqnacdntcyhblgdzhbbydmjregkzyheyybjmcdtafzjzhgcjnlghldwxjjkytcyksssmtwcttqzlpbszdtwcxgzagyktywxlnlcpbclloqmmzsslcmbjcsdzkydczjgqjdsmcytzqqlnzqzxssbpkdfqmddzzsddtdmfhtdycnaqjqkypbdjyyxtljhdrqxlmhkydhrnlklytwhllrllrcxylbnsrnzzsymqzzhhkyhxksmzsyzgcxfbnbsqlfzxxnnxkxwymsddyqnggqmmyhcdzttfgyyhgsbttybykjdnkyjbelhdypjqnfxfdnkzhqksbyjtzbxhfdsbdaswpawajldyjsfhblcnndnqjtjnchxfjsrfwhzfmdrfjyxwzpdjkzyjympcyznynxfbytfyfwygdbnzzzdnytxzemmqbsqehxfznbmflzzsrsyqjgsxwzjsprytjsjgskjjgljjynzjjxhgjkymlpyyycxycg"
        "qzswhwlyrjlpxslcxmnsmwklcdnknynpsjszhdzeptxmwywxyysywlxjqcqxzdclaeelmcpjpclwbxsqhfwrtfnjtnqjhjqdxhwlbyccfjlylkyynldxnhycstyywncjtxywtrmdrqnwqcmfjdxzmhmayxnwmyzqtxtlmrspwwjhanbxtgzypxyyrrclmpamgkqjszycymyjsnxtplnbappypylxmyzkynldgyjzcchnlmzhhanqnbgwqtzmxxmllhgdzxnhxhrxycjmffxywcfsbssqlhnndycannmtcjcypnxnytycnnymnmsxndlylysljnlxyssqmllyzlzjjjkyzzcsfbzxxmstbjgnxnchlsnmcjscyznfzlxbrnnnylmnrtgzqysatswryhyjzmgdhzgzdwybsscskxsyhytsxgcqgxzzbhyxjscrhmkkbsczjyjymkqhzjfnbhmqhysnjnzybknqmcjgqhwlsnzswxkhljhyybqcbfcdsxdldspfzfskjjzwzxsddxjseeegjscssygclxxnwwyllymwwwgydkzjggggggsycknjwnjpcxbjjtqtjwdsspjxcxnzxnmelptfsxtllxcljxjjljsxctnswxlennlyqrwhsycsqnybyaywjejqfwqcqqcjqgxaldbzzyjgkgxbltqyfxjltpydkyqhpmatlcndnkxmtxynhklefxdllegqtymsawhzmljtkynxlyjzljeeyybqqffnlyxhdsctgjhxywlkllxqkcctnhjlqmkkzgcyygllljdcgydhzwypysjbzjdzgyzzhywyfqdtyzszyezklymgjjhtsmqwyzljyywzcsrkqyqltdxwcdrjalwsqzwbdcqyncjnnszjlncdcdtlzzzacqqzzddxyblxcbqjylzllljddzjgyqyjzyxnyyyexjxksdaznyrdlzyyynjlslldyxjcykywnqcclddnyyynycgczhjxcclgzqjgnwnncqqjysbzzxyjxjnxjf"
        "zbsbdsfnsfpzxhdwztdmpptflzzbzdmyypqjrsdzsqzsqxbdgcpzswdwcsqzgmdhzxmwwfybpngphdmjthzsmmbgzmbzjcfzhfcbbnmqdfmbcmcjxlgpnjbbxgyhyyjgptzgzmqbqdcgybjxlwnkydpdymgcftpfxyztzxdzxtgkptybbclbjaskytssqyymscxfjhhlsllsjpqjjqaklyldlycctsxmcwfgngbqxllllnyxtyltyxytdpjhnhgnkbyqnfjyyzbyyessessgdyhfhwtcqbsdzjtfdmxhcnjzymqwsrxjdzjqbdqbbsdjgnfbknbxdkqhmkwjjjgdllthzhhyyyyhhsxztyyyccbdbpypzyccztjpzywcbdlfwzcwjdxxhyhlhwczxjtcnlcdpxnqczczlyxjjcjbhfxwpywxzpcdzzbdccjwjhmlxbqxxbylrddgjrrctttgqdczwmxfytmmzcwjwxyywzzkybzcccttqnhxnwxxkhkfhtswoccjybcmpzzykbnnzpbthhjdlszddytyfjpxyngfxbyqxzbhxcpxxtnzdnnycnxsxlhkmzxlthdhkghxxsshqyhhcjyxglhzxcxnhekdtgqxqypkdhentykcnymyyjmkqyyyjxzlthhqtbyqhxbmyhsqckwwyllhcyylnneqxqwmcfbdccmljggxdqktlxkknqcdgcjwyjjlyhhqyttnwchhxcxwherzjydjccdbqcdgdnyxzdhcqrxcbhztqcbxwgqwyybxhmbymykdyecmqkyaqyngyzslfnkkqgyssqyshngjctxkzycssbkyxhyylstycxqthysmnscpmmgcccccmnztasmgqzjhklosjylswtmqzyqkdzljqqyplzycztcqqpbbcjzclpkhqcyyxxdtdddsjcxffllchqxmjlwcjcxtspycxndtjshjwhdqqqckxyamylsjhmlalygxcyydmamdqmlmcznnyybzxkyflmcncmlhxrcjjhs"
        "ylnmtjggzgywjxsrxcwjgjqhqzdqjdcjjskjkgdzcgjjyjylxzxxcdqhhheslmhlfsbdjsyyshfyssczqlpbdrfnztzdkykhsccgkwtqzckmsynbcrxqbjyfaxpzzedzcjykbcjwhyjbqzzywnyszptdkzpfpbaztklqnhbbzptpptyzzybhnydcpzmmcycqmcjfzzdcmnlfpbplngqjtbttajzpzbbdnjkljqylnbzqhksjznggqstzkcxchpzsnbcgzkddzqanzgjkdrtlzldwjnjzlywtxndjzjhxnatncbgtzcsskmljpjytsnwxcfjwjjtkhtzplbhsnjssyjbhbjyzlstlsbjhdnwqpslmmfbjdwajyzccjtbnnnzwxxcdslqgdsdpdzgjtqqpsqlyyjzlgyhsdlctcbjtktyczjtqkbsjlgnnzdncsgpynjzjjyyknhrpwszxmtncszzyshbyhyzaxywkcjtllckjjtjhgcssxyqyczbynnlwqcglzgjgqyqcczssbcrbcskydznxjsqgxssjmecnstjtpbdlthzwxqwqczexnqczgwesgssbybstscslccgbfsdqnzlccglllzghzcthcnmjgyzazcmsksstzmmzckbjygqljyjppldxrkzyxccsnhshhdznlzhzjjcddcbcjxlbfqbczztpqdnnxljcthqzjgylklszzpcjdscqjhjqkdxgpbajynnsmjtzdxlcjyryynhjbngzjkmjxltbsllrzpylssznxjhllhyllqqzqlsymrcncxsljmlzltzldwdjjllnzggqxppskyggggbfzbdkmwggcxmcgdxjmcjsdycabxjdlnbcddygskydqdxdjjyxhsaqazdzfslqxxjnqzylblxxwxqqzbjzlfbblylwdsljhxjyzjwtdjcyfqzqzzdzsxzzqlzcdzfxhwspynpqzmlpplffxjjnzzylsjnyqzfpfzgsywjjjhrdjzzxtxxglghtdxcskyswmmtc"
        "wybazbjkshfhgcxmhfqhyxxyzftsjyzbxyxpzlchmzmbxhzzssyfdmncwdabazlxktcshhxkxjjzjsthygxsxyyhhhjwxkzxssbzzwhhhcwtzzzpjxsyxqqjgzyzawllcwxznxgyxyhfmkhydwsqmnjnaycyspmjkgwcqhylajgmzxhmmcnzhbhxclxdjpltxyjkdyylttxfqzhyxxsjbjnayrsmxyplckdnyhlxrlnllstycyyqygzhhsccsmcctzcxhyqfpyyrpbflfqnntszlljmhwtcjqyzwtlnmlmdwmbzzsnzrbpdddlqjjbxtcsnzqqygwcsxfwzlxccrszdzmcyggdyqsgtnnnlsmymmsyhfbjdgyxccpshxczcsbsjyygjmpbwaffyfnxhydxzylremzgzzyndsznlljcsqfnxxkptxzgxjjgbmyyssnbtylbnlhbfzdcyfbmgqrrmzszxysjtznnydzzcdgnjafjbdknzblczszpsgcycjszlmnrznbzzldlnllysxsqzqlcxzlsgkbrxbrbzcycxzjzeeyfgklzlnyhgzcgzlfjhgtgwkraajyzkzqtsshjjxdzyznynnzyrzdqqhgjzxsszbtkjbbfrtjxllfqwjgclqtymblpzdxtzagbdhzzrbgjhwnjtjxlkscfsmwlldcysjtxkzscfwjlbnntzlljzllqblcqmqqcgcdfpbphzczjlpyyghdtgwdxfczqyyyqysrclqzfklzzzgffcqnwglhjycjjczlqzzyjbjzzbpdcsnnjgxdqnknlznnnnpsntsdyfwwdjzjysxyyczcyhzwbbyhxrylybhkjksfxtjjmmchhlltnyymsxxyzpdjjycsycwmdjjkqyrhllngpngtlyycljnnnxjyzfnmlrgjjtyzbsyzmsjyjhgfzqmsyxrszcytlrtqzsstkxgqkgsptgxdnjsgcqcqhmxggztqydjjznlbznxqlhyqgggthqscbyhjhhkyygkggc"
        "mjdzllcclxqsftgjslllmlcskctbljszszmmnytpzsxqhjcnnqnyexzqzcpshkzzyzxxdfgmwqrllqxrfztlystctmjcsjjthjnxtnrztzfqrhcgllgcnnnnjdnlnnytsjtlnyxsszxcgjzyqpylfhdjsbbdczgjjjqzjqdybssllcmyttmqnbhjqmnygjyeqyqmzgcjkpdcnmyzgqllslnclmholzgdylfzslncnzlylzcjeshnyllnxnjxlyjyyyxnbcljsswcqqnnyllzldjnllzllbnylnqchxyyqoxccqkyjxxxyklksxeyqhcqkkkkcsnyxxyqxygwtjohthxpxxhsnlcykychzzcbwqbbwjqcscszsslcylgddsjzmmymcytsdsxxscjpqqsqylyfzychdjynywcbtjsydchcyddjlbdjjsodzyqyskkyxdhhgqjyohdyxwgmmmazdybbbppbcmnnpnjzsmtxerxjmhqdntpjdcbsnmssythjtslmltrcplzszmlqdsdmjmqpnqdxcfrnnfsdqqyxhyaykqyddlqyyysszbydslntfgtzqbzmchdhczcwfdxtmqqsphqwwxsrgjcwnntzcqmgwqjrjhtqjbbgwzfxjhnqfxxqywyyhyscdydhhqmrmtmwctbszppzzglmzfollcfwhmmsjzttdhlmyffytzzgzyskjjxqyjzqbhmbzclyghgfmshpcfzsnclpbqsnjyzslxxfpmtyjygbxlldlxpzjyzjyhhzcywhjylsjexfszzywxkzjlnadymlymqjpwxxhxsktqjezrpxxzghmhwqpwqlyjjqjjzszcnhjlchhnxjlqwzjhbmzyxbdhhypylhlhlgfwlcfyytlhjjcwmscpxstkpnhjxsntyxxtestjctlsslstdlllwwyhdnrjzsfgxssyczykwhtdhwjglhtzdqdjzxxqgghltzphcsqfclnjtclzpfstpdynylgmjllycqhynspchylhqyqtmzym"
        "bywrfqykjsyslzdnjmpxyyssrhzjnyqtqdfzbwwdwwrxcwggyhxmkmyyyhmxmzhnksepmlqqmtcwctmxmxjpjjhfxyyzsjzhtybmstsyjznqjnytlhynbyqclcycnzwsmylknjxlggnnpjgtysylymzskttwlgsmzsylmpwlcwxwqcssyzsyxyrhssntsrwpccpwcmhdhhxzdzyfjhgzttsbjhgyglzysmyclllxbtyxhbbzjkssdmalhhycfygmqypjyjqxjllljgclzgqlycjcctotyxmtmshllwlqfxymzmklpszzcxhkjyclctyjcyhxsgyxnnxlzwpyjpxhjwpjpwxqqxlxsdhmrslzzydwdtcxknstzshbsccstplwsscjchjlcgchssphylhfhhxjsxallnylmzdhzxylsxlmzykcldyahlcmddyspjtqjzlngjfsjshctsdszlblmssmnyymjqbjhrzwtyydchjljapzwbgqxbkfnbjdllllyylsjydwhxpsbcmljpscgbhxlqhyrljxyswxhhzlldfhlnnymjljyflyjycdrjlfsyzfsllcqyqfgqyhnszlylmdtdjcnhbzllnwlqxygyyhbmgdhxxnhlzzjzxczzzcyqzfngwpylcpkpykpmclgkdgxzgxwqbdxzzkzfbddlzxjtpjpttbythzzdwslcpnhsltjxxqlhyxxxywzyswttzkhlxzxzpyhgzhknfsyhntjrnxfjcpjztwhplshfcrhnslxxjxxyhzqdxqwnnhyhmjdbflkhcxcwhjfyjcfpqcxqxzyyyjygrpynscsnnnnchkzdyhflxxhjjbyzwttxnncyjjymswyxqrmhxzwfqsylznggbhyxnnbwttcsybhxxwxyhhxyxnknyxmlywrnnqlxbbcljsylfsytjzyhyzawlhorjmnsczjxxxyxchcyqryxqzddsjfslyltsffyxlmtyjmnnyyyxltzcsxqclhzxlwyxzhnnlrxkxjcd"
        "yhlbrlmbrdlaxksnlljlyxxlynrylcjtgncmtlzllcyzlpzpzyawnjjfybdyyzsepckzzqdqpbpsjpdyttbdbbbyndycncpjmtmlrmfmmrwyfbsjgygsmdqqqztxmkqwgxllpjgzbqrdjjjfpkjkcxbljmswldtsjxldlppbxcwkcqqbfqbccajzgmykbhyhhzykndqzybpjnspxthlfpnsygyjdbgxnhhjhzjhstrstldxskzysybmxjlxyslbzyslzxjhfybqnbylljqkygzmcyzzymccslnlhzhwfwyxzmwyxtynxjhbyymcysbmhysmydyshnyzchmjjmzcaahcbjbbhblytylsxsnxgjdhkxxtxxnbhnmlngsltxmrhnlxqqxmzllyswqgdlbjhdcgjyqyymhwfmjybbbyjyjwjmdpwhxqldyapdfxxbcgjspckrssyzjmslbzzjfljjjlgxzgyxyxlszqkxbexyxhgcxbpndyhwectwwcjmbtxchxyqqllxflyxlljlssnwdbzcmyjclwswdczpchqekcqbwlcgydblqppqzqfnqdjhymmcxtxdrmzwrhxcjzylqxdyynhyyhrslnrsywwjjymtltllgtqcjzyabtckzcjyccqlysqxalmzynywlwdnzxqdllqshgpjfjljnjabcqzdjgthhsstnyjfbswzlxjxrhgldlzrlzqzgsllllzlymxxgdzhgbdphzpbrlwnjqbpfdwonnnhlypcnjccndmbcpbzzncyqxldomzblzwpdwyygdstthcsqsccrsssyslfybnntyjszdfndpdhtqzmbqlxlcmyffgtjjqwftmnpjwdnlbzcmmcngbdzlqlpnfhyymjylsdchdcjwjcctljcldtljjcbddpndsszycndbjlggjzxsxnlycybjjxxcbylzcfzppgkcxqdzfztjjfjdjxzbnzyjqctyjwhdyczhymdjxttmpxsplzcdwslshxypzgtfmlcjtacbbmgde"
        "wycyzxdszjyhflystygwhkjyylsjcxgywjcbllcsnddbtzbsclyzczzssqdllmjyyhfllqllxfdyhabxggnywyypllsdldllbjcyxjznlhljdxyyqytdlllbngpfdfbbqbzzmdpjhgclgmjjpgaehhbwcqxajhhhzchxyphjaxhlphjpgpzjqcqzgjjzzgzdmqyybzzphyhybwhazyjhykfgdpfqsdlzmljxjpgalxzdaglmdgxmmzqwtxdxxpfdmmssympfmdmmkxksyzyshdzkjsysmmzzzmdydyzzczxbmlstmdyemxckjmztyymzmzzmsshhdccjewxxkljsthwlsqlyjzllsjssdppmhnlgjczyhmxxhgncjmdhxtkgrmxfwmckmwkdcksxqmmmszzydkmsclcmpcjmhrpxqpzdsslcxkyxtwlkjyahzjgzjwcjnxyhmmbmlgjxmhlmlgmxctkzmjlyscjsyszhsyjzjcdajzhbsdqjzgwtkqxfkdmsdjlfmnhkzqkjfeypzyszcdpynffmzqykttdzzefmzlbnpplplpbpszalltnlkckqzkgenjlwalkxydpxnhsxqnwqnkxqclhyxxmlnccwlymqyckynnlcjnszkpyzkcqzqljbdmdjhlasqlbydwqlwdgbqcryddztjybkbwszdxdtnpjdtcnqnfxqqmgnseclstbhpwslctxxlpwydzklnqgzcqapllkqcylbqmqczqcnjslqzdjxlddhpzqdljjxzqdjyzhhzlkcjqdwjppypqakjyrmpzbnmcxkllzllfqpylllmbsglzysslrsysqtmxyxzqzbscnysyztffmzzsmzqhzssccmlyxwtpzgxzjgzgsjzgkddhtqggzllbjdzlsbzhyxyzhzfywxytymsdnzzyjgtcmtnxqyxjscxhslnndlrytzlryylxqhtxsrtzcgyxbnqqzfhykmzjbzymkbpnlyzpblmcnqyzzzsjztjctzhhyzzjrdyzh"
        "nfxklfzslkgjtctssyllgzrzbbjzzklpkbczysnnyxbjfbnjzzxcdwlzyjxzzdjjgggrsnjkmsmzjlsjywqsnyhqjsxpjztnlsnshrnynjtwchglbnrjlzxwjqxqkysjycztlqzybbybyzjqdwgyzcytjcjxckcwdkkzxsnkdnywwyyjqyytlytdjlxwkcjnklccpzcqqdzzqlcsfqchqqgssmjzzllbjjzysjhtsjdysjqjpdszcdchjkjzzlpycgmzndjxbsjzzsyzyhgxcpbjydssxdzncglqmbtsfcbfdzdlznfgfjgfsmpnjqlnblgqcyyxbqgdjjqsrfkztjdhczklbsdzcfytplljgjhtxzcsszzxstjygkgckgynqxjplzbbbgcgyjzgczqszlbjlsjfzgkqqjcgycjbzqtldxrjnbsxxpzshszycfwdsjjhxmfczpfzhqhqmqnknlyhtycgfrzgnqxcgpdlbzcsczqlljblhbdcypscppdymzzxgyhckcpzjgslzlnscnsldlxbmsdlddfjmkdqdhslzxlsznpqpgjdlybdskgqlbzlnlkyyhzttmcjnqtzzfszqktlljtyyllnllqyzqlbdzlslyyzxmdfszsnxlxznczqnbbwskrfbcylctnblgjpmczzlstlxshtzcyzlzbnfmqnlxflcjlyljqcbclzjgnsstbrmhxzhjzclxfnbgxgtqncztmsfzkjmssncljkbhszjntnlzdntlmmjxgzjyjczxyhyhwrwwqnztnfjscpyshzjfyrdjsfscjzbjfzqzchzlxfxsbzqlzsgyftzdcszxzjbjpszkjrhxjzcgbjkhcggtxkjqglxbxfgtrtylxqxhdtsjxhjzjjcmzlcqsbtxwqgxtxxhxftsdkfjhzyjfjxnzldlllcqsqqzqwqxswqtwgwbzcgcllqzbclmqjtzgzyzxljfrmyzflxnsnxxjkxrmjdzdmmyxbsqbhgzmwfwygmjlzbyytgzyccd"
        "jyzxsngnyjyznbgpzjcqsyxsxrtfyzgrhztxszzthcbfclsyxzlzqmzlmplmxzjssfsbysmzqhxxnxrxhqzzzsslyflczjrcrxhhzxqndshxsjjhqcjjbcynsysxjbqjpxzqplmlxzkyxlxcnlcycxxzzlxdlllmjyhzxhyjwkjrwyhcpsgnrzlfzwfzznsxgxflzsxzzzbfcsyjdbrjkrdhhjxjljjtgxjxxstjtjxlyxqfcsgswmsbctlqzzwlzzkxjmltmjyhsddbxgzhdlbmyjfrzfcgclyjbpmlysmsxlszjqqhjzfxgfqfqbphngyyqxgztnqwyltlgwgwwhnlfmfgzjmgmgbgtjflyzzgzyzaflsspmlbflcwbjztljjmzlpjjlymqtmyyyfbgygqzglyzdxqyxrqqqhsxyyqxygjtyxfsfsllgnqcygycwfhcccfxpylypllzqxxxxxqqhhsshjzcftsczjxspzwhhhhhapylqnlpqafyhxdylnkmzqgggddesrenzltzgchyppcsqjjhclljtolnjpzljlhymhezdydsqycddhgznndzclzywllznteydgnlhslpjjbdgwxpcnntycklkclwkllcasstknzdnnjttlyyzssysszzryljqkcgdhhyrxrzydgrgcwcgzqffbppjfzynakrgywyjpqxxfkjtszzxswzddfbbqtbgtzkznpzfpzxzpjszbmqhkyyxyldkljnypkyghgdzjxxeaxpnznctzcmxcxmmjxnkszqnmnlwbwwqjjyhclstmcsxnjcxxtpcnfdtnnpglllzcjlspblpgjcdtnjjlyarscffjfqwdpgzdwmrzzcgodaxnssnyzrestyjwjyjdbcfxnmwttbqlwstszgybljpxglbnclgpcbjftmxzljylzxcltpnclcgxtfzjshcrxsfysgdkntlbyjcyjllstgqcbxnhzxbxklylhzlqzlnzcqwgzlgzjncjgcmnzzgjdzxtzjxy"
        "cyycxxjyyxjjxsssjstsstdppghtcsxwzdcsynptfbchfbblzjclzzdbxgcjlhpxnfzflsyltnwbmnjhszbmdnbcysccldnycndqlyjjhmqllcsgljjsyfpyyccyltjantjjpwycmmgqyysxdxqmzhszxbftwwzqswqrfkjlzjqqyfbrxjhhfwjgzyqacmyfrhcyybynwlpexcczsyyrlttdmqlrkmpbgmyyjprkznbbsqyxbhyzdjdnghpmfsgbwfzmfqmmbzmzdcgjlnnnxyqgmlrygqccyxzlwdkcjcggmcjjfyzzjhycfrrcmtznzxhkqgdjxccjeascrjthpljlrzdjrbcqhjdnrhylyqjsymhzydwcdfryhbbydtssccwbxglpzmlzjdqsscfjmmxjcxjytycghycjwynsxlfemwjnmkllswtxhyyyncmmcyjdqdjzglljwjnkhpzggflccsczmcbltbhbqjxqdjpdjztghglfjawbzyzjltstdhjhctcbchflqmpwdshyytqwcnntjtlnnmnndyyyxsqkxwyyflxxnzwcxypmaelyhgjwzzjbrxxaqjfllpfhhhytzzxsgqjmhspgdzqwbwpjhzjdyjcqwxkthxsqlzyymysdzgnqckknjlwpnsyscsyzlnmhqsyljxbcxtlhzqzpcycykpppnsxfyzjjrcemhszmnxlxglrwgcstlrsxbygbzgnxcnlnjlclynymdxwtzpalcxpqjcjwtcyyjlblxbzlqmyljbghdslssdmxmbdczsxyhamlczcpjmcnhjyjnsykchskqmczqdllkablwjqsfmocdxjrrlyqchjmybyqlrhetfjzfrfksryxfjdwtsxxywsqjyslyxwjhsdlxyyxhbhawhwjcxlmyljcsqlkydttxbzslfdxgxsjkhsxxybssxdpwncmrptqzczenygcxqfjxkjbdmljzmqqxnoxslyxxlylljdzptymhbfsttqqwlhsgynlzzalzxcl"
        "htwrrqhlstmypyxjjxmnsjnnbryxyjllyqyltwylqyfmlkljdnlltfzwkzhljmlhljnljnnlqxylmbhhlnlzxqchxcfxxlhyhjjgbyzzkbxscqdjqdsndzsygzhhmgsxcsymxfepcqwwrbpyyjqryqcyjhqqzyhmwffhgzfrjfcdbxntqyzpcyhhjlfrzgpbxzdbbgrqstlgdgylcqmgchhmfywlzyxkjlypjhsywmqqggzmnzjnsqxlqsyjtcbehsxfszfxzwfllbcyyjdytdthwzsfjmqqyjlmqsxlldttkghybfpwdyysqqrnqwlgwdebzwcyygcnlkjxtmxmyjsxhybrwfymwfrxyymxysctzztfykmldhqdlgyjnlcryjtlpsxxxywlsbrrjwxhqybhtydnhhxmmywytycnnmnssccdalwztcpqpyjllqzyjswjwzzmmglmxclmxnzmxmzsqtzppjqblpgxjzhfljjhycjsrxwcxsncdlxsyjdcqzxslqyclzxlzzxmxqrjmhrhzjbhmfljlmlclqnldxzlllfyprgjynxcqqdcmqjzzxhnpnxzmemmsxykynlxsxtljxyhwdcwdzhqyybgybcyscfgfsjnzdrzzxqxrzrqjjymcanhrjtldbpyzbstjhxxzypbdwfgzzrpymnnkxcqbyxnbnfyckrjjcmjegrzgyclnnzdnkknsjkcljspgyyclqqjybzssqlllkjftbgtylcccdblsppfylgydtzjqjzgkntsfcxbdkdxxhybbfytyhbclnnytgdhryrnjsbtcsnyjqhklllzslydxxwbcjqsbxnpjzjzjdzfbxxbrmladhcsnclbjdstblprznswsbxbcllxxlzdnzsjpynyxxyftnnfbhjjjgbygjpmmmmsszljmtlyzjxswxtyledqpjmpgqzjgdjlqjwjqllsdgjgygmscljjxdtygjqjjjcjzcjgdzdshqgzjggcjhqxsnjlzzbxhsgzxcxyljx"
        "yxyydfqqjhjfxdhctxjyrxysqtjxyefyyssyxjxncyzxfxcsxszxyyschshxzzzgzzzgfjdldylnpzgsjaztyqzpbxcbdztzczyxxyhhscjshcggqhjhgxhsctmzmehyxgebtclzkkwytjzrslekestdbcyhqqsayxcjxwwgsphjszsdncsjkqcxswxfctynydpccczjqtcwjqjzzzqzljzhlsbhpydxpsxshhezdxfptjqyzcxhyaxncfzyyhxgnqmywntzsjbnhhgymxmxqcnssbcqsjyxxtyyhybcqlmmszmjzzllcogxzaajzyhjmchhcxzsxsdznleyjjzjbhzwjzsqtzpsxzzdsqjjjlnyazphhyysrnqzthzhnyjyjhdzxzlswclybzyecwcycrylchzhzydzydyjdfrjjhtrsqtxyxjrjhojynxelxsfsfjzghpzsxzszdzcqzbyyklsgsjhczshdgqgxyzgxchxzjwyqwgyhksseqzzndzfkwyssdclzstsymcdhjxxyweyxczaydmpxmdsxybsqmjmzjmtjqlpjyqzcgqhyjhhhqxhlhdldjqcfdwbsxfzzyyschtytyjbhecxhjkgqfxbhyzjfxhwhbdzfyzbchpnpgdydmsxhkhhmamlnbyjtmpxejmcthqbzyfcgtyhwphftgzzezsbzegpbmdskftycmhbllhgpzjxzjgzjyxzsbbqsczzlzscstpgxmjsfdcczjzdjxsybzlfcjsazfgszlwbczzzbyztzynswyjgxzbdsynxlgzbzfygczxbzhzftpbgzgejbstgkdmfhyzzjhzllzzgjqzlsfdjsscbzgpdlfzfzszyzyzsygcxsnxxchczxtzzljfzgqsqqxcjqccccdjcdszzyqjccgxztdlgscxzsyjjqtcclqdqztqchqqyzynzzzpbkhdjfcjfztypqyqttynlmbdktjcpqzjdzfpjsbnjlgyjdxjdcqkzgqkxclbzjtcjdqbxdjjjst"
        "cxnxbxqmslyjcxntjqwwcjjnjjlllhjcwqtbzqqczczpzzdzyddcyzdzccjgtjfzdprntctjdcxtqzdtjnplzbcllctdsxkjzqdmzlbznbtjdcxfczdbczjjltqqpldckztbbzjcqdcjwynllzlzccdwllxwzlxrxntqjczxkjlsgdnqtddglnlajjtnnynkqlldzntdnycygjwyxdxfrsqstcdenqmrrqzhhqhdldazfkapbggpzrebzzykyqspeqjjglkqzzzjlysyhyzwfqznlzzlzhwcgkypqgnpgblplrrjyxcccgyhsfzfwbzywtgzxyljczwhncjzplfflgskhyjdeyxhlpllllcygxdrzelrhgklzzyhzlyqszzjzqljzflnbhgwlczcfjwspyxzlzlxgccpzbllcxbbbbnbbcbbcrnnzccnrbbnnldcgqyyqxygmqzwnzytyjhyfwtehznjywlccntzyjjcdedpwdztstnjhtymbjnyjzlxtsstphndjxxbyxqtzqddtjtdyztgwscszqflshlnzbcjbhdlyzjyckwtydylbnydsdsycctyszyyebgexhqddwnygyclxtdcystqnygzascsszzdzlcclzrqxyywljsbymxshzdembbllyyllytdqyshymrqnkfkbfxnnsbychxbwjyhtqbpbsbwdzylkgzskyghqzjxhxjxgnljkzlyycdxlfwfghljgjybxblybxqpqgntzplncybxdjyqydymrbeyjyyhkxxstmxrczzjwxyhybmcflyzhqyzfwxdbxbcwzmslpdmyckfmzklzcyqycclhxfzlydqzpzygyjyzmdxtzfnnyttqtzhgsfcdmlccytzxjcytjmkslpzhysnwllytpzctzccktxdhxxtqcyfksmqccyyazhtjplylzlyjbjxtfnyljyynrxcylmmnxjsmybcsysslzylljjgyldzdlqhfzzblfndsqkczfyhhgqmjdsxycttxnqnjpy"
        "ybfcjtyyfbnxejdgyqbjrcnfyyqpghyjsyzngrhtknlnndzntsmgklbygbpyszbydjzsstjztsxzbhbscsbzczptqfzlqflypybbjgszmnxdjmtsyskkbjtxhjcegbsmjyjzcstmljyxrczqscxxqpyzhmkyxxxjcljyrmyygadyskqlnadhrskqxzxztcggzdlmlwxybwsyctbhjhcfcwzsxwwtgzlxqshnyczjxemplsrcgltnzntlzjcyjgdtclglbllqpjmzpapxyzlaktkdwczzbncctdqqzqyjgmcdxltgcszlmlhbglkznnwzndxnhlnmkydlgxdtwcfrjerctzhydxykxhwfzcqshknmqqhzhhymjdjskhxzjzbzzxympajnmctbxlsxlzynwrtsqgscbptbsgzwyhtlkssswhzzlyytnxjgmjrnsnnnnlskztxgxlsammlbwldqhylakqcqctmycfjbslxclzjclxxknbnnzlhjphqplsxsckslnhpsfqcytxjjzljldtzjjzdlydjntptnndskjfsljhylzqqzlbthydgdjfdbyadxdzhzjnthqbyknxjjqczmlljzkspldsclbblnnlelxjlbjycxjxgcnlcqplzlznjtsljgyzdzpltqcssfdmnycxgbtjdcznbgbqyqjwgkfhtnbyqzqgbkpbbyzmtjdytblsqmbsxtbnpdxklemyycjynzdtldykzzxtdxhqshygmzsjycctayrzlpwltlkxslzcggexclfxlkjrtlqjaqzncmbqdkkcxglczjzxjhptdjjmzqykqsecqzdshhadmlzfmmzbgntjnnlhbyjbrbtmlbyjdzxlcjlpldlpcqdhlhzlycblcxccjadqlmzmmsshmybhbnkkbhrsxxjmxmdznnpklbbrhgghfchgmnklltsyyycqlcskymyehywxnxqywbawykqldnntndkhqcgdqktgpkxhcpdhtwnmssyhbwcrwxhjmkmzngwtml"
        "kfghkjyldyycxwhyyclqhkqhtdqkhffldxqwytyydesbpkyrzpjfyyzjceqdzzdlattpbfjllcxdlmjsdxegwgsjqxcfbssszpdyzcxznyxppzydlyjccpltxlnxyzyrscyyytylwwndsahjsygyhgywwaxtjzdaxysrltdpssyxfnejdxyzhlxlllzhzsjnyqyqyxyjghzgjcyjchzlycdshhsgczyjscllnxzjjyyxnfsmwfpyllyllabmddhwzxjmcxztzpmlqchsfwzynctlndywlslxhymmylmbwwkyxyaddxylldjpybpwnxjmmmllhafdllaflbnhhbqqjqzjcqjjdjtffkmmmpythygdrjrddwrqjxnbysrmzdbyytbjhpymyjtjxaahggdqtmystqxkbtzbkjlxrbyqqhxmjjbdjntgtbxpgbktlgqxjjjcdhxqdwjlwrfmjgwqhcnrxswgbtgygbwhswdwrfhwytjjxxxjyzyslphyypyyxhydqpxshxyxgskqhywbdddpplcjlhqeewjgsyykdpplfjthkjltcyjhhjttpltzzcdlyhqkcjqysteeyhkyzyxxyysddjkllpymqyhqgxqhzrhbxpllnqydqhxsxxwgdqbshyllpjjjthyjkyphthyyktyezyenmdshlzrpqfbnfxzbsftlgxsjbswyysksflxlpplbbblnsfbfyzbsjssylpbbffffsscjdstjsxtryjcyffsyzyzbjtlctsbsdhrtjjbytcxyyeylycbnebjdsysyhgsjzbxbytfzwgenhhhthjhhxfwgcstbgxklstyymtmbyxjskzscdyjrcythxzfhmymcxlznsdjtxtxrycfyjsbsdyerxhljxbbdeynjghxgckgscymblxjmsznskgxfbnbbthfjyafxwxfbxmyfhdttcxzzpxrsywzdlybbktyqwqjbzypzjznjpzjlztfysbttslmptzrtdxqsjehbnylndxljsqmlhtx"
        "tjecxalzzspktlzkqqyfsyjywpcpqfhjhytqxzkrsgtksqczlptxcdyyzsslzslxlzmacpcqbzyxhbsxlzdltztjtylzjyytbzypltxjsjxhlbmytxcqrblzssfjzztnjytxmyjhlhpblcyxqjqqkzzscpzkswalqsplczzjsxgwwwygyatjbbctdkhqhkgtgpbkqyslbxbbckbmllndzstbklggqkqlzbkktfxrmdkbftpzfrtppmferqnxgjpzsstlbztpszqzsjdhljqlzbpmsmmsxlqqnhknblrddnhxdkddjcyyljfqgzlgsygmjqjkhbpmxyxlytqwlwjcpbmjxcyzydrjbhtdjyeqshtmgsfyplwhlzffnynnhxqhpltbqpfbjwjdbygpnxtbfzjgnnntjshxeawtzylltyqbwjpgxghnnkndjtmszsqynzggnwqtfhclssgmnnnnynzqqxncjdqgzdlfnykljcjllzlmzznnnnsshthxjlzjbbhqjwwycrdhlyqqjbeyfsjhthnrnwjhwpslmssgzttygrqqwrnlalhmjtqjsmxqbjjzjqzyzkxbjqxbjxshzssfglxmxnxfghkzszggslcnnarjxhnlllmzxelglxydjytlfbkbpnlyzfbbhptgjkwetzhkjjxzxxglljlstgshjjyqlqzfkcgnndjsszfdbctwwseqfhqjbsaqtgypjlbxbmmywxgslzhglsgnyfljbyfdjfngsfmbyzhqffwjsyfyjjphzbyyzffwotjnlmftwlbzgyzqxcdjygzyyryzynyzwegazyhjjlzrthlrmgrjxzclnnnljjyhtbwjybxxbxjjtjteekhwslnnlbsfazpqqbdlqjjtyyqlyzkdksqjnejzldqcgjqnnjsncmrfqthtejmfctyhypymhydmjncfgyyxwshctxrljgjzhzcyyyjltkttntmjlzclzzayyoczlrlbszywjytsjyhbyshfjlykjxxtmzyyltxxyp"
        "slqyjzyzyypnhmymdyylblhlsyygqllnjjymsoycbzgdlyxylcqyxtszegxhzglhwbljheyxtwqmakbpqcgyshhegqcmwyywljyjhyyzlljjylhzyhmgsljljxcjjyclycjbcpzjzjmmwlcjlnqljjjlxyjmlszljqlycmmgcfmmfpqqmfxlqmcffqmmmmhnznfhhjgtthxkhslnchhyqzxtmmqdcydyxyqmyqylddcyaytazdcymdydlzfffmmycqcwzzmabtbyctdmndzggdftypcgqyttssffwbdttqssystwnjhjytsxxylbyyhhwhxgzxwznnqzjzjjqjccchykxbzszcnjtllcqxynjnckycynccqnxyewyczdcjycchyjlbtzyycqwlpgpyllgktltlgkgqbgychjxy";

    QString ret;
    for (int i=0; i<str.size(); i++) {
        wchar_t wchar = *(str.utf16()+i);
        if (0X4E00 <= wchar && wchar <= 0X9FA5)
            ret.append(wcsFiestLetterTable[wchar-0X4E00]);
        else if (wchar < 0x7f)
            ret.append((char)wchar);
    }

    return ret;
}

QString Utils::getFileMD5(QString filePath)
{
    QString result;
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        QCryptographicHash hash(QCryptographicHash::Md5);
        while (!file.atEnd())
            hash.addData(file.read(4096));
        result = hash.result().toHex().toUpper();
    }
    return result;
}

QString Utils::getMD5(QByteArray data)
{
    return QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex().toUpper();
}

QByteArray Utils::execute(QString program, QStringList args, bool *result)
{
    if (result) *result = false;

    QByteArray data;

#if defined(Q_OS_LINUX)
    QString cmd = QString("%1 %2 2>&1").arg(program, args.join(" "));
    // LOGD("begin: %s", qUtf8Printable(cmd));
    do {
        QString out;

        FILE *fp;
        char buffer[1024];

        fp = popen(qUtf8Printable(cmd), "r");
        if (fp == nullptr)
            break;

        while (fgets(buffer, sizeof(buffer), fp) != nullptr) {
            out.append(buffer);
        }

        pclose(fp);

        data = out.toUtf8();

        if (result) *result = true;
    } while (false);
    // LOGD("end: %s", qUtf8Printable((cmd));
#elif !defined(Q_OS_WASM) && !defined(Q_OS_IOS)
    QProcess p;
    p.setProgram(program);
    p.setArguments(args);
    p.setProcessChannelMode(QProcess::MergedChannels);
    p.start();
    if (p.waitForFinished()) {
        data = p.readAll();

        if (result) *result = true;
    }
    else {
        LOGC("exec(%s %s): %s",
                  qUtf8Printable(program),
                  qUtf8Printable(args.join(' ')),
                  qUtf8Printable(p.errorString()));
    }
#endif

    return data;
}

bool Utils::isFileUrl(QString url)
{
    if (!url.contains("://"))
        return true;

    if (!url.startsWith("http"))
        return false;

    QUrl u(url);
    return u.path().contains(".");
}

int Utils::startEventLoop(QEventLoop *event_loop, int timeout)
{
    QTimer::singleShot(timeout, event_loop, [event_loop](){
        event_loop->quit();
    });
    return event_loop->exec();
}

QTemporaryFile *Utils::createFileCopies(QString filename)
{
    QTemporaryFile *ret = nullptr;
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly)) {

        // dump data
        ret = new QTemporaryFile;
        if (ret->open()) {
            file.seek(0);
            char buffer[1024];
            while (true) {
                qint64 len = file.read(buffer, 1024);
                if (len < 1)
                    break;
                ret->write(buffer, len);
            }
            ret->seek(0);
        } else {
            delete ret;
            ret = nullptr;
        }

        file.close();
    }

    return ret;
}

#ifdef QT_GUI_LIB
QByteArray Utils::imageToBase64(QByteArray data, QString format)
{
    QByteArray imageData(QString("data:image/%1;base64,").arg(format).toUtf8());
    imageData.append(data.toBase64());
    return imageData;
}

QByteArray Utils::imageToBase64(QImage img)
{
    if (img.isNull())
        return QByteArray();

    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    if (!img.save(&buffer, "JPG"))
        return QByteArray();

    buffer.seek(0);
    QByteArray jpg_data = buffer.readAll();

    return imageToBase64(jpg_data, "jpeg");
}

QImage Utils::base64ToImage(QByteArray data)
{
    // e.g. data:image/png;base64,<content>
    QByteArray imageData = QByteArray::fromBase64(data.mid(data.indexOf(",")+1));
    QImage image;
    image.loadFromData(imageData);
    return image;
}
#endif

QString Utils::stringToBase64(QString str)
{
    return str.toUtf8().toBase64();
}

QString Utils::base64ToString(QString str)
{
    return QByteArray::fromBase64(str.toLocal8Bit());
}

void Utils::recycleMemory()
{
#ifdef Q_OS_ANDROID

#elif defined(Q_OS_LINUX)
#ifdef HAVE_TCMALLOC
    MallocExtension::instance()->ReleaseFreeMemory();
#else
    malloc_trim(0);
#endif
#endif

#ifdef Q_OS_WIN
    // 获取当前进程句柄
    HANDLE hProcess = GetCurrentProcess();
    // 设置工作集大小
    SetProcessWorkingSetSize(hProcess, -1, -1);
    // 关闭进程句柄
    CloseHandle(hProcess);
#endif
}

void Utils::snapshootMemory()
{
#ifdef Q_OS_ANDROID

#elif defined(Q_OS_LINUX)
    malloc_stats();
#endif

#ifdef HAVE_TCMALLOC
    HeapProfilerDump("Tcmalloc Dump!");
#endif
}

QString Utils::applicationName()
{
    QString filename;
#ifdef Q_OS_LINUX
    filename = Utils::execute(QString::asprintf("readlink /proc/%d/exe", getpid()));
    filename = filename.trimmed();
#elif defined(Q_OS_WIN)
    char szFileName[MAX_PATH + 1] = { 0 };
    ::GetModuleFileNameA(NULL, szFileName, MAX_PATH);
    auto gbk = QTextCodec::codecForName("gbk");
    filename = gbk->toUnicode(szFileName);
#endif

    QFileInfo fi(filename);
    return fi.baseName();
}

QString Utils::applicationDirPath()
{
    QString filename;
#ifdef Q_OS_LINUX
    filename = Utils::execute(QString::asprintf("readlink /proc/%d/exe", getpid()));
#elif defined(Q_OS_WIN)
    char szFileName[MAX_PATH + 1] = { 0 };
    ::GetModuleFileNameA(NULL, szFileName, MAX_PATH);
    auto gbk = QTextCodec::codecForName("gbk");
    filename = gbk->toUnicode(szFileName);
#endif

    QFileInfo fi(filename);
    return fi.absolutePath();
}

#ifdef Q_OS_WIN
QString GetWmiValue(const std::string& query, const std::string& property)
{
    QString result;
    HRESULT hres;

    // 初始化 COM
    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres))
    {
        return result;
    }

    // 初始化 COM安全
    hres = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE,
        NULL);

    // 初始化 WMI
    IWbemLocator* pLoc = NULL;
    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&pLoc);

    IWbemServices* pSvc = NULL;
    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),
        NULL,
        NULL,
        0,
        NULL,
        0,
        0,
        &pSvc);

    // 设置安全级别
    hres = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE,
        NULL);

    // 查询
    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t(query.c_str()),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);

    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;

    while (pEnumerator)
    {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
        if (0 == uReturn)
            break;

        VARIANT vtProp;
        hres = pclsObj->Get(_bstr_t(property.c_str()), 0, &vtProp, 0, 0);
        if (SUCCEEDED(hres))
        {
            result = _com_util::ConvertBSTRToString(vtProp.bstrVal);
            VariantClear(&vtProp);
        }
        pclsObj->Release();
    }

    // 清理
    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();

    return result.trimmed();
}
#endif

QString Utils::getMachineId()
{
    static QString matchine_id;
    if (matchine_id.isEmpty()) {
#ifdef Q_OS_WIN
        QString cpu_id = GetWmiValue("SELECT ProcessorId FROM Win32_Processor", "ProcessorId");
        QString motherboard_d = GetWmiValue("SELECT SerialNumber FROM Win32_BaseBoard", "SerialNumber");
        QString hard_disk_id = GetWmiValue("SELECT SerialNumber FROM Win32_DiskDrive", "SerialNumber");

        QByteArray data = QString("%1-%2-%3").arg(cpu_id, motherboard_d, hard_disk_id).toUtf8();
        matchine_id = getMD5(data);
#else
        matchine_id = Utils::execute("cat /etc/machine-id").trimmed();
#endif
    }
    return matchine_id;
}

void Utils::addToStartup(QString name, QString filename)
{
#ifdef Q_OS_WIN
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {
        filename = filename.replace("/", "\\");
        filename.prepend("\"");
        filename.append("\"");
        QByteArray value = filename.toLocal8Bit().data();
        LONG result = RegSetValueExA(hKey, name.toLocal8Bit().data(), 0, REG_SZ, (const BYTE *)value.data(), value.size());
        if (result != ERROR_SUCCESS)
            LOGW("RegSetValue error: %s", qUtf8Printable(name));
        RegCloseKey(hKey);
    }
#endif
}

void Utils::delFromStartup(QString name)
{
#ifdef Q_OS_WIN
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {
        LONG result = RegDeleteValueA(hKey, name.toLocal8Bit().data());
        if (result != ERROR_SUCCESS)
            LOGW("RegDeleteKey error: %s", qUtf8Printable(name));
        RegCloseKey(hKey);
    }
#endif
}

namespace {
class InitUtils {
public:
    InitUtils() {
        // Utils::getMachineId();
    }
};
InitUtils s_instance;
}
