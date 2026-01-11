import { NextResponse } from 'next/server';
import fs from 'fs';
import path from 'path';

export async function GET(request: Request) {
    try {
        const dataDir = path.join(process.cwd(), 'data', 'tracks');

        if (!fs.existsSync(dataDir)) {
            return NextResponse.json({ tracks: [] });
        }

        const files = fs.readdirSync(dataDir).filter(file => file.endsWith('.json'));
        const tracks = [];

        for (const file of files) {
            try {
                const filePath = path.join(dataDir, file);
                const content = fs.readFileSync(filePath, 'utf8');
                const data = JSON.parse(content);

                // Format for device
                // Device expects: name, lat, lon (center/start), configs
                tracks.push({
                    name: data.name,
                    lat: data.startLine.lat,
                    lon: data.startLine.lng,
                    heading: data.startLine.bearing, // Heading for start line
                    width: data.startLine.width,
                    configs: ["Default"] // Simplify for now
                });
            } catch (e) {
                console.error(`Error reading track file ${file}:`, e);
            }
        }

        return NextResponse.json({
            success: true,
            tracks: tracks
        });

    } catch (error) {
        console.error('Error listing tracks:', error);
        return NextResponse.json(
            { error: 'Internal Server Error' },
            { status: 500 }
        );
    }
}
