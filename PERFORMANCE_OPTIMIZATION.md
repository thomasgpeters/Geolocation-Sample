# Performance Optimization Guide

This document details the performance optimizations implemented in the FranchiseAI application, focusing on the AI Search functionality and external API integrations.

## Overview

The AI Search feature relies on external APIs (OpenStreetMap Overpass, Nominatim, Google APIs) which can introduce significant latency. The optimizations below target query efficiency, network performance, and failure handling to provide responsive user feedback.

## Overpass API Optimizations

The Overpass API is used to query OpenStreetMap data for business prospects. This is typically the slowest part of the search pipeline.

### Bounding Box vs Radius Queries

**Problem:** The `around:` radius query is computationally expensive.

```
// SLOW - computes distance from center to every element
node["office"](around:16000,39.7392,-104.9903);
```

**Solution:** Use bounding box (`bbox`) queries instead.

```
// FAST - simple coordinate bounds check using spatial index
[bbox:39.62,−105.11,39.86,−104.87]
node["office"];
```

**Impact:** 5-10x faster query execution

**Implementation:** `OpenStreetMapAPI.cpp:buildCateringProspectQuery()`
```cpp
// Convert radius to bounding box
double latDelta = limitedRadiusKm / 111.0;
double lonDelta = limitedRadiusKm / (111.0 * std::cos(lat * 3.14159265 / 180.0));

double south = lat - latDelta;
double north = lat + latDelta;
double west = lon - lonDelta;
double east = lon + lonDelta;

query << "[out:json][timeout:6][bbox:" << south << "," << west << "," << north << "," << east << "];";
```

### Combined Node/Way Queries

**Problem:** Separate queries for nodes and ways double the query clauses.

```
// SLOW - 10 separate clauses
node["office"]["name"](around:...);
way["office"]["name"](around:...);
node["tourism"="hotel"](around:...);
way["tourism"="hotel"](around:...);
// ... etc
```

**Solution:** Use `nw` shorthand to query both in one clause.

```
// FAST - 5 combined clauses
nw["office"]["name"];
nw["tourism"="hotel"];
// ... etc
```

**Impact:** 50% fewer query clauses

### Quadtile Output Sorting

**Problem:** Default output ordering requires full result processing.

**Solution:** Add `qt` modifier for quadtile-sorted output.

```cpp
query << "out center qt " << maxResults << ";";
```

**Impact:** Faster result streaming, limited result count

### Faster Overpass Endpoint

**Problem:** Main overpass-api.de server can be slow under load.

**Solution:** Use the lz4 mirror which provides compressed responses.

```cpp
std::string overpassEndpoint = "https://lz4.overpass-api.de/api/interpreter";
```

**Impact:** Compressed responses, often faster server response

## Network Optimizations

### HTTP Compression

**Problem:** Large JSON responses increase transfer time.

**Solution:** Accept compressed responses.

```cpp
curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip, deflate");
```

**Impact:** Smaller payload sizes, faster transfers

### TCP Optimizations

**Problem:** Default TCP settings add latency for small requests.

**Solution:** Enable TCP_NODELAY and TCP_KEEPALIVE.

```cpp
// Disable Nagle algorithm - send small packets immediately
curl_easy_setopt(curl, CURLOPT_TCP_NODELAY, 1L);

// Enable keepalive for faster failure detection
curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
```

**Impact:** Lower latency for request/response cycles

## Timeout Configuration

Aggressive timeouts provide fast feedback when services are slow or unavailable.

### Current Timeout Settings

| Service | Connect Timeout | Request Timeout | Location |
|---------|----------------|-----------------|----------|
| Overpass API | 3s | 8s | `OSMAPIConfig` |
| Nominatim | 3s | 8s | `OSMAPIConfig` |
| Geocoding Service | 2s | 5s | `GeocodingConfig` |
| Google Places | 3s | 8s | `GooglePlacesConfig` |
| Google Geocoding | 3s | 5s | `GoogleGeocodingConfig` |

### Timeout Strategy

1. **Connection timeout:** How long to wait for TCP connection. Should be short (2-3s) since connection establishment is fast on healthy networks.

2. **Request timeout:** Total time for the entire request. Should account for:
   - Connection time
   - Server processing
   - Response transfer

3. **Query timeout (Overpass):** Server-side timeout in the query itself.
   ```cpp
   query << "[out:json][timeout:6]...";
   ```

### Configuration

```cpp
// OpenStreetMapAPI.h
struct OSMAPIConfig {
    std::string overpassEndpoint = "https://lz4.overpass-api.de/api/interpreter";
    int requestTimeoutMs = 8000;    // 8 seconds
    int connectTimeoutMs = 3000;    // 3 seconds
    // ...
};

// GeocodingService.h
struct GeocodingConfig {
    int requestTimeoutMs = 5000;    // 5 seconds
    int connectTimeoutMs = 2000;    // 2 seconds
    // ...
};
```

## Caching Strategy

### Response Caching

OSM data is relatively static, so aggressive caching is beneficial.

```cpp
int cacheDurationMinutes = 1440;  // 24 hours
```

**Cache key format:**
```cpp
std::string cacheKey = std::to_string(latitude) + "," +
                       std::to_string(longitude) + "," +
                       std::to_string(radiusMeters);
```

### Known Locations Cache

Common US cities are pre-cached for instant geocoding:

- New York, Los Angeles, Chicago, Houston, Phoenix
- Philadelphia, San Antonio, San Diego, Dallas, San Jose
- Austin, Jacksonville, Fort Worth, Columbus, Charlotte
- Seattle, Denver, Boston, Nashville, Detroit, etc.

## Query Scope Limiting

### Radius Limiting

Large search areas cause slow queries and excessive results.

```cpp
// Max ~8 miles (13 km) for performance
double limitedRadiusKm = std::min(radiusMeters / 1000.0, 13.0);
```

### Result Limiting

Limit results to prevent memory issues and slow processing.

```cpp
int maxResultsPerQuery = 50;
query << "out center qt " << maxResultsPerQuery << ";";
```

## Prospect Type Focus

Query only high-value catering prospects to reduce result set:

| OSM Tag | Business Type |
|---------|---------------|
| `office` + `name` | Named offices |
| `office=company` | Company offices |
| `office=corporation` | Corporate offices |
| `tourism=hotel` | Hotels |
| `amenity=conference_centre` | Conference centers |
| `amenity=hospital` | Hospitals |
| `amenity=university` | Universities |
| `amenity=college` | Colleges |

## Error Handling for Performance

### Fast Failure

Don't wait for slow services - fail fast and try alternatives.

```cpp
// Short connection timeout for fast failure
curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 3000);
```

### Fallback Chain

Geocoding uses a fallback chain for reliability:

1. Google Geocoding (if configured) - fastest, most reliable
2. Nominatim (OpenStreetMap) - free, no API key
3. Known locations cache - instant for common cities
4. Default location (Denver, CO) - guaranteed result

## Performance Comparison

### Before Optimization

| Metric | Value |
|--------|-------|
| Typical search time | 10-30 seconds |
| Query timeout | 30 seconds |
| Query type | `around:` radius |
| Clauses per query | 10+ |
| Compression | None |

### After Optimization

| Metric | Value |
|--------|-------|
| Typical search time | 1-3 seconds |
| Query timeout | 6 seconds |
| Query type | `bbox` bounding box |
| Clauses per query | 5 |
| Compression | gzip/deflate |

## Monitoring Performance

### Search Duration Tracking

The search results include timing information:

```cpp
auto startTime = std::chrono::high_resolution_clock::now();
// ... perform search ...
auto endTime = std::chrono::high_resolution_clock::now();
results.searchDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
    endTime - startTime
);
```

### API Call Statistics

```cpp
osmAPI.getTotalApiCalls();
osmAPI.getCacheSize();
```

## Future Optimization Opportunities

1. **Connection pooling:** Reuse HTTP connections across requests
2. **Parallel queries:** Query multiple Overpass endpoints simultaneously
3. **Progressive loading:** Show results as they arrive
4. **Predictive caching:** Pre-cache likely search areas
5. **WebSocket connections:** Reduce connection overhead for frequent queries
6. **Local OSM extract:** Use local data for specific regions

## Configuration Reference

### OSMAPIConfig (OpenStreetMapAPI.h)

```cpp
struct OSMAPIConfig {
    std::string overpassEndpoint = "https://lz4.overpass-api.de/api/interpreter";
    std::string nominatimEndpoint = "https://nominatim.openstreetmap.org";
    int requestTimeoutMs = 8000;
    int connectTimeoutMs = 3000;
    bool enableCaching = true;
    int cacheDurationMinutes = 1440;
    int maxResultsPerQuery = 50;
    std::string userAgent = "FranchiseAI/1.0";
};
```

### GeocodingConfig (GeocodingService.h)

```cpp
struct GeocodingConfig {
    GeocodingProvider provider = GeocodingProvider::NOMINATIM;
    std::string apiKey;
    std::string endpoint;
    int requestTimeoutMs = 5000;
    int connectTimeoutMs = 2000;
    bool enableCaching = true;
    int cacheDurationMinutes = 1440;
    std::string userAgent = "FranchiseAI/1.0";
    int maxRequestsPerSecond = 1;
};
```
