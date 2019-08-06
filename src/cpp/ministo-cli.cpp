#include <iostream>

#include "hybrid_ministo.h"


/**
 * Ministo (Namespace)
 */
namespace ministo {
    /* Initialize "hybrid" ministo miner. */
    // ::HybridMinisto* hybrid_ministo = nullptr;

    /* Call C++ dtors: */
    // void cleanup(void* p) {
    //     delete reinterpret_cast<HybridMinisto*>(p);
    // }


    /**
     * Ministo (Class)
     */
    // class Ministo : public AsyncWorker {
    //     public:
    //         Ministo(Callback *callback)
    //         : AsyncWorker(callback)
    //         { }
    //
    //         ~Ministo() { }
    //
    //         // NOTE: This function runs in a thread spawned by NAN.
    //         void Execute () {
    //             if (hybrid_ministo) {
    //                 hybrid_ministo->run(); // blocking call
    //             } else {
    //                 SetErrorMessage("{ error: 'NO hybrid_ministo available!' }");
    //             }
    //         }
    //
    //     private:
    //         /**
    //          * Executed when the async work is complete
    //          * this function will be run inside the main event loop
    //          * so it is safe to use V8 again.
    //          */
    //         void HandleOKCallback () {
    //             HandleScope scope;
    //
    //             v8::Local<v8::Value> argv[] = {
    //                 Null(),
    //                 New<v8::String>(hybrid_ministo->solution()).ToLocalChecked()
    //             };
    //
    //             /* Make the call. */
    //             Call(callback->GetFunction(), GetCurrentContext()->Global(), 2, argv);
    //         }
    // };

    /**
     * Run the MAIN asynchronous process.
     *
     * NOTE: First and only parameter is a callback function
     * receiving the solution when found.
     */
    // NAN_METHOD(run) {
    //     /* Initialize callback. */
    //     Callback *callback = new Callback(To<v8::Function>(info[0]).ToLocalChecked());
    //
    //     /* Start Ministo worker. */
    //     AsyncQueueWorker(new Ministo(callback));
    // }

    /**
     * Stop the MAIN asynchronous process.
     */
    // NAN_METHOD(stop) {
    //     /* Stop Ministo. */
    //     hybrid_ministo->stop();
    //
    //     info.GetReturnValue().SetUndefined();
    // }

    /**
     * Set Hardware Type
     */
    // NAN_METHOD(setHardwareType) {
    //     /* Retrieve info. */
    //     MaybeLocal<v8::String> inp = Nan::To<v8::String>(info[0]);
    //
    //     /* Validate info. */
    //     if (!inp.IsEmpty()) {
    //         /* Set info. */
    //         hybrid_ministo->setHardwareType(std::string(*Nan::Utf8String(inp.ToLocalChecked())));
    //     }
    //
    //     info.GetReturnValue().SetUndefined();
    // }

    /**
     * Set Thread Size
     */
    // NAN_METHOD(setThreadSize) {
    //     /* Retrieve info. */
    //     MaybeLocal<v8::String> inp = Nan::To<v8::String>(info[0]);
    //
    //     /* Validate info. */
    //     if (!inp.IsEmpty()) {
    //         hybrid_ministo->setThreadSize(std::string(*Nan::Utf8String(inp.ToLocalChecked())));
    //     }
    //
    //     info.GetReturnValue().SetUndefined();
    // }

    /**
     * Set Block Size
     */
    // NAN_METHOD(setBlockSize) {
    //     /* Retrieve info. */
    //     MaybeLocal<v8::String> inp = Nan::To<v8::String>(info[0]);
    //
    //     /* Validate info. */
    //     if (!inp.IsEmpty()) {
    //         hybrid_ministo->setBlockSize(std::string(*Nan::Utf8String(inp.ToLocalChecked())));
    //     }
    //
    //     info.GetReturnValue().SetUndefined();
    // }

    /**
     * Set Challenge (Number)
     */
    // NAN_METHOD(setChallenge) {
    //     /* Retrieve info. */
    //     MaybeLocal<v8::String> inp = Nan::To<v8::String>(info[0]);
    //
    //     /* Validate info. */
    //     if (!inp.IsEmpty()) {
    //         hybrid_ministo->setChallenge(std::string(*Nan::Utf8String(inp.ToLocalChecked())));
    //     }
    //
    //     info.GetReturnValue().SetUndefined();
    // }

    /**
     * Set (Difficulty) Target
     */
    // NAN_METHOD(setTarget) {
    //     /* Retrieve info. */
    //     MaybeLocal<v8::String> inp = Nan::To<v8::String>(info[0]);
    //
    //     /* Validate info. */
    //     if (!inp.IsEmpty()) {
    //         hybrid_ministo->setTarget(std::string(*Nan::Utf8String(inp.ToLocalChecked())));
    //     }
    //
    //     info.GetReturnValue().SetUndefined();
    // }

    /**
     * Set Ministo Address
     */
    // NAN_METHOD(setMinterAddress) {
    //     /* Retrieve info. */
    //     MaybeLocal<v8::String> inp = Nan::To<v8::String>(info[0]);
    //
    //     /* Validate info. */
    //     if (!inp.IsEmpty()) {
    //         hybrid_ministo->setMinterAddress(std::string(*Nan::Utf8String(inp.ToLocalChecked())));
    //     }
    //
    //     info.GetReturnValue().SetUndefined();
    // }

    /**
     * Get Number of Hashes (performed until now),
     * then reset it to 0.
     *
     * TODO: Need to make one of these for the gpu solver.. ?
     */
    // NAN_METHOD(hashes) {
    //     uint32_t const value = CPUSolver::hashes;
    //
    //     CPUSolver::hashes = 0;
    //
    //     info.GetReturnValue().Set(value);
    // }
}

int main()
{
    /* Welcome. */
    std::cout << "Ministo CLI" << std::endl;

    /* Initialize instance. */
    // HybridMinisto hybrid_ministo;
    HybridMinisto* hybrid_ministo = nullptr;

    if (hybrid_ministo) {
        std::cout << "Starting hybrid_ministo run!" << std::endl;
        hybrid_ministo->run(); // blocking call
    } else {
        std::cout << "NO hybrid_ministo available!" << std::endl;
    }

    /* Test solution. */
    // std::cout << "Solution -> " << hybrid_ministo.solution() << std::endl;

    return 0;
}
