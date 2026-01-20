"use client";

import {
    LifeBuoy,
    Mail,
    Map,
    BookOpen,
    ChevronLeft,
    Youtube
} from "lucide-react";
import Link from "next/link";
import { useRouter } from "next/navigation";

export default function HelpPage() {
    const router = useRouter();

    return (
        <main className="min-h-screen bg-background text-foreground pb-safe">
            {/* Header */}
            <div className="flex items-center justify-between p-4 bg-background z-10 sticky top-0">
                <button onClick={() => router.back()} className="p-2 -ml-2">
                    <ChevronLeft className="w-8 h-8 text-primary" />
                </button>
                <h1 className="text-lg font-medium text-text-secondary">Help</h1>
                <div className="w-8"></div> {/* Spacer */}
            </div>

            <div className="p-6 text-center space-y-8">

                {/* Banner */}
                <div className="space-y-4">
                    <h2 className="text-primary font-bold tracking-widest uppercase">Info & Support</h2>
                    <p className="text-text-secondary text-sm leading-relaxed px-4">
                        We maintain a large list of tutorial videos on YouTube. Check it out <a href="#" className="text-primary font-bold">here</a>
                    </p>
                </div>

                {/* Grid */}
                <div className="grid grid-cols-2 gap-6">
                    <HelpCard icon={LifeBuoy} label="FAQ" href="#" />
                    <HelpCard icon={Mail} label="Contact us" href="#" />
                    <HelpCard icon={Map} label="Tracks Database" href="/tracks" />
                    <HelpCard icon={BookOpen} label="RaceBox User Manual" href="#" />
                    <HelpCard icon={Youtube} label="Tutorials" href="#" />
                </div>

            </div>
        </main>
    );
}

function HelpCard({ icon: Icon, label, href }: { icon: any, label: string, href: string }) {
    return (
        <Link href={href} className="flex flex-col items-center justify-center gap-4 p-8 bg-card-bg rounded-xl border border-border-color hover:bg-white/5 transition-colors aspect-square">
            <Icon className="w-16 h-16 text-primary stroke-[1.5]" />
            <span className="font-bold text-foreground text-sm">{label}</span>
        </Link>
    );
}
