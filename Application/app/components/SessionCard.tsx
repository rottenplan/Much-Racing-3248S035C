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
        <div className="carbon-bg p-4 rounded-xl shadow-md border border-border-color flex items-center justify-between mb-3 cursor-pointer hover:border-primary/50 hover:shadow-lg transition-all group">
            <div className="flex flex-col gap-1">
                <span className="text-text-secondary text-xs font-medium uppercase tracking-wide">{date}</span>
                <h3 className="text-foreground font-racing text-base group-hover:text-primary transition-colors">{title}</h3>
                {type === "track" ? (
                    <div className="flex items-center gap-4 text-sm text-text-secondary">
                        <span>{subtitle}</span>
                    </div>
                ) : (
                    <div className="text-sm text-text-secondary">{subtitle}</div>
                )}
            </div>

            <div className="flex items-center gap-3">
                {highlight && (
                    <div className="text-right">
                        {highlightLabel && <div className="text-xs text-text-secondary font-medium">{highlightLabel}</div>}
                        <div className={`text-xl font-data font-bold ${highlightColor === 'green' ? 'text-highlight' :
                            highlightColor === 'red' ? 'text-primary' : 'text-foreground'
                            }`}>
                            {highlight}
                        </div>
                    </div>
                )}
                <ChevronRight className="text-primary w-5 h-5 group-hover:translate-x-1 transition-transform" />
            </div>
        </div>
    );

    if (href) {
        return <Link href={href}>{content}</Link>;
    }

    return content;
}
