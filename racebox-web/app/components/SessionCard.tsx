import { ChevronRight } from "lucide-react";
import Link from "next/link";

interface SessionCardProps {
    date: string;
    title: string;
    subtitle: string;
    highlight?: string; // e.g. "01:24.71" or "5.45"
    highlightLabel?: string; // e.g. "Best Lap" or "Best 100-200"
    highlightColor?: "green" | "red" | "normal";
    type: "track" | "drag";
    href?: string;
}

export default function SessionCard({
    date,
    title,
    subtitle,
    highlight,
    highlightLabel,
    highlightColor = "normal",
    type,
    href,
}: SessionCardProps) {
    const content = (
        <div className="bg-card-bg p-4 rounded-xl shadow-sm border border-gray-100 flex items-center justify-between mb-3 cursor-pointer hover:bg-gray-50 transition-colors">
            <div className="flex flex-col gap-1">
                <span className="text-gray-500 text-sm font-medium">{date}</span>
                <h3 className="text-foreground font-bold text-lg">{title}</h3>
                {type === "track" ? (
                    <div className="flex items-center gap-4 text-sm text-gray-600">
                        <span>{subtitle}</span>
                    </div>
                ) : (
                    <div className="text-sm text-gray-600">{subtitle}</div>
                )}
            </div>

            <div className="flex items-center gap-3">
                {highlight && (
                    <div className="text-right">
                        {highlightLabel && <div className="text-xs text-gray-400">{highlightLabel}</div>}
                        <div className={`text-lg font-bold ${highlightColor === 'green' ? 'text-highlight' :
                            highlightColor === 'red' ? 'text-primary' : 'text-foreground'
                            }`}>
                            {highlight}
                        </div>
                    </div>
                )}
                <ChevronRight className="text-gray-300 w-5 h-5" />
            </div>
        </div>
    );

    if (href) {
        return <Link href={href}>{content}</Link>;
    }

    return content;
}
