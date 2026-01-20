
import { XMLParser } from 'fast-xml-parser';

export interface GpxPoint {
    lat: number;
    lon: number;
    ele?: number;
    time?: string;
    speed?: number; // Extensions often contain speed
    extensions?: any;
}

export interface ParsedTrack {
    name?: string;
    points: GpxPoint[];
    metadata?: any;
}

export function parseGpx(gpxContent: string): ParsedTrack {
    const parser = new XMLParser({
        ignoreAttributes: false,
        attributeNamePrefix: "@_"
    });

    const result = parser.parse(gpxContent);
    const trk = result.gpx?.trk;
    const trkseg = trk?.trkseg;

    // Handle single or array of segments/points
    let points: any[] = [];
    if (Array.isArray(trkseg)) {
        trkseg.forEach((seg: any) => {
            if (Array.isArray(seg.trkpt)) {
                points = points.concat(seg.trkpt);
            } else if (seg.trkpt) {
                points.push(seg.trkpt);
            }
        });
    } else if (trkseg?.trkpt) {
        if (Array.isArray(trkseg.trkpt)) {
            points = trkseg.trkpt;
        } else {
            points = [trkseg.trkpt];
        }
    }

    const cleanPoints: GpxPoint[] = points.map((pt: any) => ({
        lat: parseFloat(pt['@_lat']),
        lon: parseFloat(pt['@_lon']),
        ele: pt.ele ? parseFloat(pt.ele) : 0,
        time: pt.time,
        // Attempt to extract speed if available in extensions (e.g. from RaceBox or other loggers)
        speed: pt.extensions?.['gpxtpx:TrackPointExtension']?.['gpxtpx:speed']
            ? parseFloat(pt.extensions['gpxtpx:TrackPointExtension']['gpxtpx:speed'])
            : 0
    }));

    return {
        name: trk?.name || "Untitled Track",
        points: cleanPoints,
        metadata: result.gpx?.metadata
    };
}
