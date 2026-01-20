"use client";

import dynamic from "next/dynamic";

const SessionMap = dynamic(() => import("./SessionMap"), {
    ssr: false,
    loading: () => <div className="w-full h-full bg-background-secondary animate-pulse flex items-center justify-center text-text-secondary text-xs">Loading Track Map...</div>
});

export default SessionMap;
