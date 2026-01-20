"use client";

import {
    Car,
    Database,
    Map,
    HelpCircle,
    Settings,
    User,
    ChevronRight,
    Ruler,
    Zap
} from "lucide-react";
import Link from "next/link";
import BottomNav from "../components/BottomNav";

export default function MorePage() {
    return (
        <main className="min-h-screen pb-24 bg-background text-foreground">
            {/* Header with Carbon Fiber */}
            <div className="carbon-bg p-4 border-b border-border-color backdrop-blur-md sticky top-0 z-10">
                <div className="flex items-center justify-between">
                    <h1 className="text-xl font-racing tracking-wide">MORE</h1>
                    <Zap className="w-6 h-6 text-primary animate-pulse" />
                </div>
            </div>

            <div className="p-4 space-y-6">

                {/* Feature Section */}
                <div className="space-y-3">
                    <h2 className="text-xs font-racing text-text-secondary uppercase tracking-wider px-2">FEATURES</h2>
                    <div className="carbon-bg rounded-xl border border-border-color overflow-hidden">
                        <MenuItem
                            icon={Database}
                            label="Track Database"
                            desc="Access a growing library of over 2000 tracks with multiple layouts."
                            href="/tracks"
                        />
                        <MenuItem
                            icon={Car}
                            label="Garage"
                            desc="Add multiple vehicles and assign them to sessions."
                            href="/garage"
                        />
                        <MenuItem
                            icon={Map}
                            label="Custom Tracks"
                            desc="Create open, closed, or hill-climb tracks on the go."
                            href="/tracks/custom"
                        />
                    </div>
                </div>

                {/* Settings Section */}
                <div className="space-y-3">
                    <h2 className="text-xs font-racing text-text-secondary uppercase tracking-wider px-2">SETTINGS & ACCOUNT</h2>
                    <div className="carbon-bg rounded-xl border border-border-color overflow-hidden">
                        <MenuItem
                            icon={User}
                            label="Account"
                            desc="Edit your account information and preferences."
                            href="/account"
                        />
                        <MenuItem
                            icon={Ruler}
                            label="Units"
                            desc="Switch between Metric and Imperial units."
                            href="/settings/units"
                        />
                        <MenuItem
                            icon={HelpCircle}
                            label="Help Section"
                            desc="Access user manuals, feature overviews, and FAQs."
                            href="/help"
                        />
                    </div>
                </div>

                {/* App Info */}
                <div className="text-center py-6 text-text-secondary">
                    <div className="text-sm font-racing">RACEBOX APP</div>
                    <div className="text-xs font-data">v2.1.0 (Build 520)</div>
                </div>

            </div>

            <BottomNav />
        </main>
    );
}

function MenuItem({ icon: Icon, label, desc, href }: { icon: any, label: string, desc: string, href: string }) {
    return (
        <Link href={href} className="flex items-center gap-4 p-4 border-b border-border-color last:border-0 hover:bg-card-bg-hover transition-colors group">
            <div className="p-2 bg-background-secondary rounded-lg border border-border-color group-hover:border-primary/50 transition-colors">
                <Icon className="w-6 h-6 text-primary" />
            </div>
            <div className="flex-1">
                <div className="font-racing text-foreground text-base group-hover:text-primary transition-colors">{label}</div>
                <div className="text-xs text-text-secondary leading-tight mt-1">{desc}</div>
            </div>
            <ChevronRight className="w-5 h-5 text-primary group-hover:translate-x-1 transition-transform" />
        </Link>
    );
}
