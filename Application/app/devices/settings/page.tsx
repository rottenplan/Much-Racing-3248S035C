'use client';

import {
    ChevronLeft,
    Edit2,
    Battery,
    MapPin,
    Signal,
} from 'lucide-react';
import Link from 'next/link';

export default function DeviceSettingsPage() {
    return (
        <div className="min-h-screen bg-background text-foreground pb-24">
            {/* Header */}
            <div className="p-4 flex items-center justify-between">
                <Link href="/devices">
                    <ChevronLeft className="w-8 h-8 text-primary" />
                </Link>
                <div className="text-center">
                    <h1 className="text-xl font-bold">21424574</h1>
                    <p className="text-text-secondary text-sm">No Default Vehicle</p>
                </div>
                <button className="p-2">
                    <Edit2 className="w-6 h-6 text-primary" />
                </button>
            </div>

            <div className="px-4 space-y-6">
                {/* Info Strip */}
                <div className="py-4 border-b border-border-color border-t mt-2">
                    <div className="flex items-center justify-between mb-1">
                        <span className="text-lg font-medium text-text-secondary">S/N: 21424574</span>
                        <span className="text-lg font-medium text-text-secondary">Model: R01</span>
                    </div>
                    <div className="text-lg font-medium text-text-secondary">FW: 1.8</div>
                </div>

                {/* Status Card */}
                <div className="bg-card-bg rounded-xl p-6 shadow-sm border border-border-color">
                    <div className="flex items-start justify-between mb-4">
                        <span className="text-highlight font-bold text-lg">Connected</span>

                        <div className="flex items-center gap-3 text-text-secondary">
                            <Battery className="w-6 h-6 text-highlight" />
                            <MapPin className="w-5 h-5" />
                            <Signal className="w-5 h-5" />
                        </div>
                    </div>

                    <div className="flex items-center justify-between text-text-secondary">
                        <span className="text-sm">All Sessions Synced</span>
                        <span className="font-mono">1.321 m / 11 sat</span>
                    </div>
                </div>

                {/* Action Buttons */}
                <div className="space-y-4 pt-2">
                    <button className="w-full py-4 text-center rounded-full border border-border-color bg-card-bg font-bold text-foreground hover:bg-white/5 transition-colors">
                        Manage Tracks
                    </button>
                    <button className="w-full py-4 text-center rounded-full border border-border-color bg-card-bg font-bold text-foreground hover:bg-white/5 transition-colors">
                        Re-sync All Sessions
                    </button>
                    <button className="w-full py-4 text-center rounded-full border border-border-color bg-card-bg font-bold text-foreground hover:bg-white/5 transition-colors">
                        Erase All Sessions
                    </button>
                    <button className="w-full py-4 text-center rounded-full border border-border-color bg-card-bg font-bold text-foreground hover:bg-white/5 transition-colors">
                        Disconnect
                    </button>
                </div>
            </div>
        </div>
    );
}
