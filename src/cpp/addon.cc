/*********************************************************************
 * NAN - Native Abstractions for Node.js
 *
 * Copyright (c) 2017 NAN contributors
 *
 * MIT License <https://github.com/nodejs/nan/blob/master/LICENSE.md>
 ********************************************************************/

#include <nan.h>

#include "hybrid_ministo.h"


/**
 * Ministo (Namespace)
 */
namespace ministo {
    /* Use NAN namespace. */
    using namespace Nan;

    /* Initialize "hybrid" ministo miner. */
    ::HybridMinisto* hybrid_ministo = nullptr;

    /* Call C++ dtors: */
    void cleanup(void* p) {
        delete reinterpret_cast<HybridMinisto*>(p);
    }


    /**
     * Ministo (Class)
     */
    class Ministo : public AsyncWorker {
        public:
            Ministo(Callback *callback)
            : AsyncWorker(callback)
            { }

            ~Ministo() { }

            // NOTE: This function runs in a thread spawned by NAN.
            void Execute () {
                if (hybrid_ministo) {
                    hybrid_ministo->run(); // blocking call
                } else {
                    SetErrorMessage("{ error: 'NO hybrid_ministo available!' }");
                }
            }

        private:
            /**
             * Executed when the async work is complete
             * this function will be run inside the main event loop
             * so it is safe to use V8 again.
             */
            void HandleOKCallback () {
                HandleScope scope;

                v8::Local<v8::Value> argv[] = {
                    Null(),
                    New<v8::String>(hybrid_ministo->solution()).ToLocalChecked()
                };

                /* Make the call. */
                Call(callback->GetFunction(), GetCurrentContext()->Global(), 2, argv);
            }
    };

    /**
     * Run the MAIN asynchronous process.
     *
     * NOTE: First and only parameter is a callback function
     * receiving the solution when found.
     */
    NAN_METHOD(run) {
        /* Initialize callback. */
        Callback *callback = new Callback(To<v8::Function>(info[0]).ToLocalChecked());

        /* Start Ministo worker. */
        AsyncQueueWorker(new Ministo(callback));
    }

    /**
     * Stop the MAIN asynchronous process.
     */
    NAN_METHOD(stop) {
        /* Stop Ministo. */
        hybrid_ministo->stop();

        info.GetReturnValue().SetUndefined();
    }

    /**
     * Set Hardware Type
     */
    NAN_METHOD(setHardwareType) {
        /* Retrieve info. */
        MaybeLocal<v8::String> inp = Nan::To<v8::String>(info[0]);

        /* Validate info. */
        if (!inp.IsEmpty()) {
            /* Set info. */
            hybrid_ministo->setHardwareType(std::string(*Nan::Utf8String(inp.ToLocalChecked())));
        }

        info.GetReturnValue().SetUndefined();
    }

    /**
     * Set Thread Size
     */
    NAN_METHOD(setThreadSize) {
        /* Retrieve info. */
        MaybeLocal<v8::String> inp = Nan::To<v8::String>(info[0]);

        /* Validate info. */
        if (!inp.IsEmpty()) {
            hybrid_ministo->setThreadSize(std::string(*Nan::Utf8String(inp.ToLocalChecked())));
        }

        info.GetReturnValue().SetUndefined();
    }

    /**
     * Set Block Size
     */
    NAN_METHOD(setBlockSize) {
        /* Retrieve info. */
        MaybeLocal<v8::String> inp = Nan::To<v8::String>(info[0]);

        /* Validate info. */
        if (!inp.IsEmpty()) {
            hybrid_ministo->setBlockSize(std::string(*Nan::Utf8String(inp.ToLocalChecked())));
        }

        info.GetReturnValue().SetUndefined();
    }

    /**
     * Set Challenge (Number)
     */
    NAN_METHOD(setChallenge) {
        /* Retrieve info. */
        MaybeLocal<v8::String> inp = Nan::To<v8::String>(info[0]);

        /* Validate info. */
        if (!inp.IsEmpty()) {
            hybrid_ministo->setChallenge(std::string(*Nan::Utf8String(inp.ToLocalChecked())));
        }

        info.GetReturnValue().SetUndefined();
    }

    /**
     * Set (Difficulty) Target
     */
    NAN_METHOD(setTarget) {
        /* Retrieve info. */
        MaybeLocal<v8::String> inp = Nan::To<v8::String>(info[0]);

        /* Validate info. */
        if (!inp.IsEmpty()) {
            hybrid_ministo->setTarget(std::string(*Nan::Utf8String(inp.ToLocalChecked())));
        }

        info.GetReturnValue().SetUndefined();
    }

    /**
     * Set Ministo Address
     */
    NAN_METHOD(setMinterAddress) {
        /* Retrieve info. */
        MaybeLocal<v8::String> inp = Nan::To<v8::String>(info[0]);

        /* Validate info. */
        if (!inp.IsEmpty()) {
            hybrid_ministo->setMinterAddress(std::string(*Nan::Utf8String(inp.ToLocalChecked())));
        }

        info.GetReturnValue().SetUndefined();
    }

    /**
     * Get Number of Hashes (performed until now),
     * then reset it to 0.
     *
     * TODO: Need to make one of these for the gpu solver.. ?
     */
    NAN_METHOD(hashes) {
        uint32_t const value = CPUSolver::hashes;

        CPUSolver::hashes = 0;

        info.GetReturnValue().Set(value);
    }

    /**
     * Defines the functions our add-on will export.
     */
    NAN_MODULE_INIT(Init) {
        Set(target
            , New<v8::String>("run").ToLocalChecked()
            , New<v8::FunctionTemplate>(run)->GetFunction()
        );

        Set(target
            , New<v8::String>("stop").ToLocalChecked()
            , New<v8::FunctionTemplate>(stop)->GetFunction()
        );

        Set(target
            , New<v8::String>("setBlockSize").ToLocalChecked()
            , New<v8::FunctionTemplate>(setBlockSize)->GetFunction()
        );

        Set(target
            , New<v8::String>("setThreadSize").ToLocalChecked()
            , New<v8::FunctionTemplate>(setThreadSize)->GetFunction()
        );

        Set(target
            , New<v8::String>("setHardwareType").ToLocalChecked()
            , New<v8::FunctionTemplate>(setHardwareType)->GetFunction()
        );

        Set(target
            , New<v8::String>("setChallenge").ToLocalChecked()
            , New<v8::FunctionTemplate>(setChallenge)->GetFunction()
        );

        Set(target
            , New<v8::String>("setTarget").ToLocalChecked()
            , New<v8::FunctionTemplate>(setTarget)->GetFunction()
        );

        Set(target
            , New<v8::String>("setMinterAddress").ToLocalChecked()
            , New<v8::FunctionTemplate>(setMinterAddress)->GetFunction()
        );

        Set(target
            , New<v8::String>("hashes").ToLocalChecked()
            , New<v8::FunctionTemplate>(hashes)->GetFunction()
        );

        hybrid_ministo = new HybridMinisto;

        node::AtExit(cleanup, hybrid_ministo);
    }

    NODE_MODULE(hybrid_ministo, Init)
}
