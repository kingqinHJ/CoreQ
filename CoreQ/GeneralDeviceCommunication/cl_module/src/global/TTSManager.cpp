#include "TTSManager.h"
#include "common/WorkerThread.h"
#include "qml/QmlUtils.h"

#include <sapi.h>
#include <sphelper.h>

class TTSManagerPrivate
{
public:
    bool abort = true;
    QString error_string;

    WorkerThread worker_thread;
};

TTSManager *TTSManager::self = nullptr;
TTSManager::TTSManager(QObject *parent) : QObject(parent)
{
    d.reset(new TTSManagerPrivate);
    d->worker_thread.setObjectName("TTS");
    self = this;
}

TTSManager::~TTSManager()
{
    stop();
    d.reset();
    self = nullptr;
    LOG_THIS();
}

void TTSManager::start()
{
    d->abort = false;
    d->worker_thread.runOnWorkerThread([](){
        CoInitialize(NULL);
    });
}

void TTSManager::stop()
{
    d->abort = true;
    d->worker_thread.runOnWorkerThread([](){
        CoUninitialize();
    });
    d->worker_thread.waitForDone();
}

bool TTSManager::create(const QString &filename, const QString &content)
{
    bool state = false;
    d->worker_thread.runOnWorkerThread([=, &state](){
        ISpVoice *pVoice = NULL;
        HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void **)&pVoice);

        if (SUCCEEDED(hr)) {
            std::shared_ptr<void> voice_guard(NULL, [&pVoice](void*){
                pVoice->Release();
            });

            CComPtr <ISpStream> cpWavStream;
            CComPtr <ISpStreamFormat> cpOldStream;
            CSpStreamFormat originalFmt;
            pVoice->GetOutputStream(&cpOldStream);
            if (cpOldStream.p == NULL) {
                d->error_string = tr("预读失败，请检查系统是否支持TTS");
                LOGW("CoCreateInstance failed: Check that the system supports TTS.");
                return;
            }

            originalFmt.AssignFormat(cpOldStream);
            // WAVEFORMATEX fmt = *originalFmt.WaveFormatExPtr();
            // fmt.nSamplesPerSec = 8000;
            // fmt.wBitsPerSample = 16;
            // originalFmt.AssignFormat(&fmt);

            hr = SPBindToFile((wchar_t *)filename.utf16(), SPFM_CREATE_ALWAYS, &cpWavStream, &originalFmt.FormatId(), originalFmt.WaveFormatExPtr());
            if (SUCCEEDED(hr)) {
                pVoice->SetOutput(cpWavStream, TRUE);

                LOGD("Speak begin: %s", qUtf8Printable(content));
                hr = pVoice->Speak((wchar_t *)content.utf16(), SPF_IS_XML, NULL);
                if (SUCCEEDED(hr))
                    LOGD("Speak end: success");
                else
                    LOGC("Speak end: error");
                state = SUCCEEDED(hr);

                if (!state)
                    d->error_string = tr("生成失败");
            }
            else {
                d->error_string = tr("生成失败，无法输出到文件");
                LOGW("SPBindToFile failed: %s", qUtf8Printable(filename));
            }
        }
        else {
            d->error_string = tr("预读失败，请检查系统是否支持TTS");
            LOGW("CoCreateInstance failed: Check that the system supports TTS.");
        }
    }, true);

    return state;
}

bool TTSManager::preread(const QString &content)
{
    bool state = false;
    d->worker_thread.runOnWorkerThread([=, &state](){
        ISpVoice *pVoice = NULL;
        HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void **)&pVoice);
        if (SUCCEEDED(hr)) {
            std::shared_ptr<void> voice_guard(NULL, [&pVoice](void*){
                pVoice->Release();
            });

            if (false) {
                ISpObjectToken *cpVoiceToken = nullptr;
                // HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech\\Voices\\Tokens\\TTS_MS_ZH-CN_HUIHUI_11.0
                // HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech\\Voices\\Tokens\\TTS_MS_EN-US_DAVID_11.0
                // HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech\\Voices\\Tokens\\TTS_MS_EN-US_ZIRA_11.0
                // HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech\\Voices\\Tokens\\TTS_MS_DE-DE_HEDDA_11.0
                hr = SpCreateNewToken(L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Speech\\Voices\\Tokens\\TTS_MS_DE-DE_HEDDA_11.0", &cpVoiceToken);
                if (SUCCEEDED(hr))
                    pVoice->SetVoice(cpVoiceToken);
                cpVoiceToken->Release();
            }

            // pVoice->SetVolume [0, 100]
            // pVoice->SetRate [-10, 10]

            LOGD("Speak begin: %s", qUtf8Printable(content));
            hr = pVoice->Speak((wchar_t *)content.utf16(), SPF_IS_XML, NULL);
            if (SUCCEEDED(hr))
                LOGD("Speak end: success");
            else
                LOGC("Speak end: error");
            state = SUCCEEDED(hr);

            if (!state)
                d->error_string = tr("预读失败");
        }
        else {
            d->error_string = tr("预读失败，请检查系统是否支持TTS");
            LOGW("CoCreateInstance failed: Check that the system supports TTS.");
        }
    }, true);

    return state;
}

QString TTSManager::errorString() const
{
    return d->error_string;
}

void TTSManager::create(const QString &filename, const QString &content, QJSValue callback)
{
    int callback_id = QmlUtils::storeCallback(callback);
    d->worker_thread.runOnWorkerThread([this, filename, content, callback_id]() {
        bool state = create(filename, content);
        QmlUtils::invokeMethod(this, callback_id, {state, d->error_string});
    });
}

void TTSManager::preread(const QString &content, QJSValue callback)
{
    int callback_id = QmlUtils::storeCallback(callback);
    d->worker_thread.runOnWorkerThread([this, content, callback_id]() {
        bool state = preread(content);
        QmlUtils::invokeMethod(this, callback_id, {state, d->error_string});
    });
}
