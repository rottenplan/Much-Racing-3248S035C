"use client";

import { Gauge, Map, MonitorSmartphone, Menu } from "lucide-react";
import Link from "next/link";
import { usePathname } from "next/navigation";

export default function BottomNav() {
    // Mock active state handling or use usePathname if actually routing
    // For this prototype we validly link to pages.

    return (
        <div className="fixed bottom-0 left-0 right-0 bg-white border-t border-gray-200 h-16 flex items-center justify-around pb-safe">
            <NavItem icon={<Gauge />} label="DRAG" href="/" />
            <NavItem icon={<Map />} label="TRACK" href="/track" />
            <NavItem icon={<MonitorSmartphone />} label="DEVICES" href="/devices" />
            <NavItem icon={<Menu />} label="MORE" href="/more" />
        </div>
    );
}

function NavItem({ icon, label, href }: { icon: React.ReactNode; label: string; href: string }) {
    // Basic active check (can be improved)
    // const pathname = usePathname();
    // const isActive = pathname === href;
    const isActive = false; // hardcode for now or use hook

    return (
        <Link href={href} className={`flex flex-col items-center gap-1 ${isActive ? "text-primary" : "text-gray-400"}`}>
            <div className="w-6 h-6">{icon}</div>
            <span className="text-[10px] font-bold tracking-wide">{label}</span>
        </Link>
    );
}
