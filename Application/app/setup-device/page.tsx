'use client';

import { useState, useEffect } from 'react';
import { useRouter } from 'next/navigation';
import Link from 'next/link';
import { Zap, Usb, CheckCircle, AlertCircle, Loader2, ArrowRight } from 'lucide-react';

export default function SetupDevicePage() {
    const router = useRouter();
    const [step, setStep] = useState<'connect' | 'detecting' | 'success' | 'error'>('connect');
    const [deviceInfo, setDeviceInfo] = useState<{ sn: string; model: string } | null>(null);
    const [errorMsg, setErrorMsg] = useState('');

    // Web Serial API Type Declaration
    // content is intentionally omitted for brevity as it's standard browser API

    const connectDevice = async () => {
        setErrorMsg('');

        // Check if Web Serial API is supported
        if (!('serial' in navigator)) {
            setErrorMsg('Web Serial API not supported in this browser. Please use Chrome or Edge.');
            return;
        }

        try {
            // Request access to the port
            // @ts-ignore
            const port = await navigator.serial.requestPort();

            setStep('detecting');

            // Open the port
            await port.open({ baudRate: 115200 });

            // In a real scenario, we would read from the stream here.
            // For now, we simulate a "handshake" or reading the S/N line.

            // Simulate detection delay
            setTimeout(() => {
                // Mock detected device
                const mockDevice = {
                    sn: 'MR-' + Math.floor(Math.random() * 1000000).toString().padStart(6, '0'),
                    model: 'RaceBox Pro'
                };

                setDeviceInfo(mockDevice);
                setStep('success');

                // Close port cleanup would happen here
                port.close().catch(console.error);

            }, 2000);

        } catch (err: any) {
            console.error('Connection failed:', err);
            // User cancelled or error
            if (err.name === 'NotFoundError') {
                // Just ignore if user cancelled
                return;
            }
            setErrorMsg('Failed to connect to device. Please try again.');
        }
    };

    const handleContinue = () => {
        // Here we would typically save the device association to the user's account via API
        // For now, we just redirect to dashboard
        router.push('/dashboard');
    };

    const handleManualSkip = () => {
        router.push('/dashboard');
    };

    return (
        <div className="min-h-screen bg-background flex flex-col items-center justify-center p-4">
            <div className="w-full max-w-md">

                {/* Header */}
                <div className="text-center mb-8">
                    <h1 className="text-3xl font-racing text-foreground mb-2">SETUP DEVICE</h1>
                    <p className="text-text-secondary">Connect your RaceBox to synchronize</p>
                </div>

                {/* Card */}
                <div className="carbon-bg border border-border-color rounded-xl p-8 shadow-2xl relative overflow-hidden">

                    {/* Step: Connect */}
                    {step === 'connect' && (
                        <div className="text-center space-y-6">
                            <div className="w-20 h-20 bg-background-secondary rounded-full flex items-center justify-center mx-auto border border-border-color">
                                <Usb className="w-10 h-10 text-primary" />
                            </div>

                            <div>
                                <h2 className="text-xl font-racing text-foreground mb-2">CONNECT VIA USB</h2>
                                <p className="text-text-secondary text-sm">
                                    Plug your device into this computer using a USB cable, then click the button below.
                                </p>
                            </div>

                            {errorMsg && (
                                <div className="bg-warning/10 border border-warning/30 rounded-lg p-3 flex items-center gap-2 text-warning text-sm text-left">
                                    <AlertCircle className="w-5 h-5 flex-shrink-0" />
                                    {errorMsg}
                                </div>
                            )}

                            <button
                                onClick={connectDevice}
                                className="w-full bg-primary hover:bg-primary-hover text-white font-racing py-4 rounded-lg transition shadow-lg hover:shadow-xl flex items-center justify-center gap-2"
                            >
                                <Zap className="w-5 h-5" />
                                CONNECT DEVICE
                            </button>

                            <button
                                onClick={handleManualSkip}
                                className="text-text-secondary hover:text-foreground text-sm underline underline-offset-4"
                            >
                                Skip setup for now
                            </button>
                        </div>
                    )}

                    {/* Step: Detecting */}
                    {step === 'detecting' && (
                        <div className="text-center space-y-8 py-8">
                            <div className="relative">
                                <div className="absolute inset-0 bg-primary/20 blur-xl rounded-full animate-pulse"></div>
                                <Loader2 className="w-16 h-16 text-primary mx-auto animate-spin relative z-10" />
                            </div>

                            <div>
                                <h2 className="text-xl font-racing text-foreground animate-pulse">DETECTING DEVICE...</h2>
                                <p className="text-text-secondary text-sm mt-2">
                                    Please wait while we identify your hardware.
                                </p>
                            </div>
                        </div>
                    )}

                    {/* Step: Success */}
                    {step === 'success' && deviceInfo && (
                        <div className="text-center space-y-6">
                            <div className="w-20 h-20 bg-highlight/10 rounded-full flex items-center justify-center mx-auto border border-highlight/30">
                                <CheckCircle className="w-10 h-10 text-highlight" />
                            </div>

                            <div>
                                <h2 className="text-xl font-racing text-highlight mb-1">DEVICE DETECTED!</h2>
                                <p className="text-text-secondary text-sm">Your device is ready to sync.</p>
                            </div>

                            <div className="bg-background-secondary border border-border-color rounded-lg p-4 text-left">
                                <div className="flex justify-between items-center mb-2">
                                    <span className="text-text-secondary text-xs">Model</span>
                                    <span className="text-foreground font-racing">{deviceInfo.model}</span>
                                </div>
                                <div className="flex justify-between items-center">
                                    <span className="text-text-secondary text-xs">Serial Number</span>
                                    <span className="text-primary font-data font-bold tracking-wider">{deviceInfo.sn}</span>
                                </div>
                            </div>

                            <button
                                onClick={handleContinue}
                                className="w-full bg-highlight hover:bg-highlight-hover text-white font-racing py-4 rounded-lg transition shadow-lg hover:shadow-xl flex items-center justify-center gap-2"
                            >
                                CONTINUE TO DASHBOARD
                                <ArrowRight className="w-5 h-5" />
                            </button>
                        </div>
                    )}

                </div>

                {/* Footer */}
                <div className="mt-8 text-center">
                    <p className="text-text-secondary text-xs">
                        Having trouble? <Link href="/help" className="text-primary hover:underline">View connection guide</Link>
                    </p>
                </div>

            </div>
        </div>
    );
}
