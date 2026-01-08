import { NextResponse } from 'next/server';
import type { NextRequest } from 'next/server';

export function middleware(request: NextRequest) {
    const token = request.cookies.get('auth_token')?.value;
    const path = request.nextUrl.pathname;

    // Define protected routes
    const protectedRoutes = ['/dashboard', '/tracks', '/sessions', '/device'];

    // Check if current path starts with any protected route
    const isProtected = protectedRoutes.some(route => path.startsWith(route));

    if (isProtected && !token) {
        const response = NextResponse.redirect(new URL('/login', request.url));
        return response;
    }

    return NextResponse.next();
}

export const config = {
    matcher: [
        '/dashboard/:path*',
        '/tracks/:path*',
        '/sessions/:path*',
        '/device/:path*',
    ],
};
