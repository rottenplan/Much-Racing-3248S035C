import { NextResponse } from 'next/server';
import fs from 'fs';
import path from 'path';

const statusFilePath = path.join(process.cwd(), 'data', 'status.json');

// Ensure data directory exists
const dataDir = path.join(process.cwd(), 'data');
if (!fs.existsSync(dataDir)) {
    fs.mkdirSync(dataDir);
}

export async function GET() {
    try {
        if (fs.existsSync(statusFilePath)) {
            const fileContent = fs.readFileSync(statusFilePath, 'utf-8');
            const data = JSON.parse(fileContent);
            return NextResponse.json(data);
        } else {
            // Default mock data if no sync has happened yet
            return NextResponse.json({
                storage_used: 0,
                storage_total: 0,
                last_sync: null
            });
        }
    } catch (error) {
        return NextResponse.json({ error: 'Failed to read status' }, { status: 500 });
    }
}
