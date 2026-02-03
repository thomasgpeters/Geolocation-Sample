-- ============================================================================
-- FranchiseAI Prospect Discovery Platform - Seed Data
-- Run this AFTER schema.sql to populate initial data
-- ============================================================================

DO $$ BEGIN RAISE NOTICE '=== Starting FranchiseAI Seed Data ==='; END $$;

-- ============================================================================
-- COMPANY TYPES (9 records)
-- Classification of companies/franchises using the FranchiseAI platform
-- ============================================================================

DO $$ BEGIN RAISE NOTICE 'Inserting COMPANY_TYPES...'; END $$;

INSERT INTO company_types (id, code, name, description, naics_prefix, default_search_radius_miles, default_prospect_industries, is_active) VALUES
-- Franchise Companies (Food Service, Retail, Services)
('c7000000-0000-0000-0000-000000000001'::uuid, 'franchise_food', 'Food Service Franchise',
 'Food service franchises including catering, restaurants, and food delivery. Target businesses that need regular food services for employees, meetings, and events.',
 '7225', 15.0, ARRAY['Corporate Offices', 'Technology Companies', 'Healthcare Facilities', 'Law Firms', 'Financial Services'], true),

('c7000000-0000-0000-0000-000000000002'::uuid, 'franchise_retail', 'Retail Franchise',
 'Retail franchises including convenience stores, specialty retail, and service retail. Target high-traffic areas and complementary businesses.',
 '44', 10.0, ARRAY['Shopping Centers', 'Office Parks', 'Residential Areas'], true),

('c7000000-0000-0000-0000-000000000003'::uuid, 'franchise_services', 'Service Franchise',
 'Service-based franchises including cleaning, maintenance, staffing, and business services. Target businesses needing ongoing service contracts.',
 '56', 25.0, ARRAY['Corporate Offices', 'Healthcare Facilities', 'Manufacturing Plants', 'Educational Institutions'], true),

-- Manufacturing Companies
('c7000000-0000-0000-0000-000000000010'::uuid, 'manufacturing_industrial', 'Industrial Manufacturing',
 'Industrial manufacturing companies producing machinery, equipment, and components. Target businesses needing industrial supplies and equipment.',
 '33', 50.0, ARRAY['Manufacturing Plants', 'Construction Companies', 'Utilities', 'Mining Operations'], true),

('c7000000-0000-0000-0000-000000000011'::uuid, 'manufacturing_consumer', 'Consumer Goods Manufacturing',
 'Consumer goods manufacturers producing food, beverages, household products, and consumer electronics. Target retailers and distributors.',
 '31', 100.0, ARRAY['Retail Stores', 'Wholesale Distributors', 'E-commerce Companies'], true),

('c7000000-0000-0000-0000-000000000012'::uuid, 'manufacturing_tech', 'Technology Manufacturing',
 'Technology and electronics manufacturing including semiconductors, computers, and telecommunications equipment. Target tech companies and system integrators.',
 '334', 75.0, ARRAY['Technology Companies', 'Data Centers', 'Telecommunications', 'Defense Contractors'], true),

-- Real Estate Companies
('c7000000-0000-0000-0000-000000000020'::uuid, 'real_estate_commercial', 'Commercial Real Estate',
 'Commercial real estate companies dealing with office buildings, retail spaces, and industrial properties. Target businesses looking for commercial space.',
 '531', 30.0, ARRAY['Growing Businesses', 'Startups', 'Expanding Corporations', 'Retail Chains'], true),

('c7000000-0000-0000-0000-000000000021'::uuid, 'real_estate_residential', 'Residential Real Estate',
 'Residential real estate companies and brokerages. Target individuals and families in relocation or home-buying markets.',
 '531', 20.0, ARRAY['Corporate HR Departments', 'Relocation Services', 'New Developments'], true),

('c7000000-0000-0000-0000-000000000022'::uuid, 'real_estate_property_mgmt', 'Property Management',
 'Property management companies handling residential and commercial properties. Target property owners and HOAs.',
 '531', 25.0, ARRAY['Property Owners', 'HOAs', 'Investment Groups', 'REITs'], true);

DO $$ BEGIN RAISE NOTICE '  -> Inserted 9 company types'; END $$;


-- ============================================================================
-- APPLICATION CONFIGURATION (13 records)
-- Feature flags, display settings, and business rules
-- ============================================================================

DO $$ BEGIN RAISE NOTICE 'Inserting APP_CONFIG...'; END $$;

-- Feature Flags (5 records)
INSERT INTO app_config (config_key, config_value, config_type, category, description, is_sensitive, is_required, default_value) VALUES
('enable_ai_search', 'true', 'boolean', 'features', 'Enable AI-powered prospect search', false, false, 'true'),
('enable_demographics', 'true', 'boolean', 'features', 'Enable demographics visualization', false, false, 'true'),
('enable_osm_data', 'true', 'boolean', 'features', 'Enable OpenStreetMap data integration', false, false, 'true'),
('enable_prospect_scoring', 'true', 'boolean', 'features', 'Enable AI-based prospect scoring', false, false, 'true'),
('enable_market_analysis', 'true', 'boolean', 'features', 'Enable AI market analysis for prospects', false, false, 'true');

-- Display/UI Settings (5 records)
INSERT INTO app_config (config_key, config_value, config_type, category, description, is_sensitive, is_required, default_value) VALUES
('default_map_zoom', '12', 'integer', 'display', 'Default zoom level for map views', false, false, '12'),
('default_search_radius', '5', 'integer', 'display', 'Default search radius in miles', false, false, '5'),
('results_per_page', '25', 'integer', 'display', 'Number of results per page in lists', false, false, '25'),
('map_tile_provider', 'openstreetmap', 'string', 'display', 'Map tile provider: openstreetmap, mapbox', false, false, 'openstreetmap'),
('theme', 'light', 'string', 'display', 'UI theme: light, dark', false, false, 'light');

-- Business Rules (4 records)
INSERT INTO app_config (config_key, config_value, config_type, category, description, is_sensitive, is_required, default_value) VALUES
('min_prospect_score', '50', 'integer', 'business', 'Minimum score to qualify as a prospect', false, false, '50'),
('hot_lead_threshold', '80', 'integer', 'business', 'Score threshold for hot lead classification', false, false, '80'),
('geocoding_cache_minutes', '1440', 'integer', 'business', 'Cache duration for geocoding results (minutes)', false, false, '1440'),
('max_search_results', '100', 'integer', 'business', 'Maximum results returned from search', false, false, '100');

-- System Settings (2 records)
INSERT INTO app_config (config_key, config_value, config_type, category, description, is_sensitive, is_required, default_value) VALUES
('current_franchisee_id', 'c2c5af5a-53a5-4d28-8218-3675c0942ead', 'string', 'system', 'Currently selected franchisee ID', false, false, ''),
('current_store_id', 'c14a9f57-2ed2-4e30-9834-98614465ddbb', 'string', 'system', 'Currently selected store location ID', false, false, '');

DO $$ BEGIN RAISE NOTICE '  -> Inserted 16 app_config records (5 features, 5 display, 4 business, 2 system)'; END $$;


-- ============================================================================
-- INDUSTRIES (15 records)
-- Industry classification with catering/service relevance scoring
-- ============================================================================

DO $$ BEGIN RAISE NOTICE 'Inserting INDUSTRIES...'; END $$;

INSERT INTO industries (name, naics_code, catering_potential_score, typical_order_size, peak_seasons) VALUES
('Corporate Offices', '551114', 9, '$500-$2000', ARRAY['Q4', 'Holiday Season']),
('Technology Companies', '541512', 9, '$800-$3000', ARRAY['Q4', 'Summer']),
('Healthcare Facilities', '621111', 8, '$300-$1500', ARRAY['Year-round']),
('Law Firms', '541110', 8, '$400-$1500', ARRAY['Q4']),
('Financial Services', '523110', 8, '$600-$2500', ARRAY['Q1', 'Q4']),
('Educational Institutions', '611310', 7, '$200-$1000', ARRAY['Fall', 'Spring']),
('Manufacturing Plants', '332710', 7, '$400-$1200', ARRAY['Year-round']),
('Government Agencies', '921110', 6, '$300-$1000', ARRAY['Q3', 'Q4']),
('Retail Stores', '452210', 5, '$150-$500', ARRAY['Holiday Season']),
('Restaurants', '722511', 3, '$100-$300', ARRAY['Off-peak catering']),
-- Additional industries for manufacturing and real estate company types
('Construction Companies', '236220', 6, '$300-$800', ARRAY['Spring', 'Summer']),
('Wholesale Distributors', '423000', 7, '$400-$1200', ARRAY['Year-round']),
('Data Centers', '518210', 8, '$500-$1500', ARRAY['Year-round']),
('Property Management', '531311', 5, '$200-$600', ARRAY['Q1', 'Q4']),
('Real Estate Agencies', '531210', 6, '$300-$900', ARRAY['Spring', 'Summer']);

DO $$ BEGIN RAISE NOTICE '  -> Inserted 15 industries'; END $$;


-- ============================================================================
-- TAGS (12 records)
-- Flexible tagging system for prospects
-- ============================================================================

DO $$ BEGIN RAISE NOTICE 'Inserting TAGS...'; END $$;

INSERT INTO tags (name, color, description) VALUES
('hot-lead', '#e74c3c', 'High priority prospect requiring immediate attention'),
('decision-maker-contact', '#27ae60', 'Has direct contact with decision maker'),
('large-events', '#3498db', 'Known for hosting large events'),
('recurring-potential', '#9b59b6', 'Potential for recurring orders'),
('competitor-customer', '#f39c12', 'Currently using competitor services'),
('referral', '#1abc9c', 'Came through referral'),
('needs-follow-up', '#e67e22', 'Requires follow-up action'),
('price-sensitive', '#95a5a6', 'Budget-conscious prospect'),
-- Additional tags for manufacturing and real estate
('high-volume', '#2980b9', 'High volume potential customer'),
('long-term-contract', '#8e44ad', 'Potential for long-term contract'),
('new-construction', '#16a085', 'New construction or development'),
('expansion-planned', '#d35400', 'Business planning expansion');

DO $$ BEGIN RAISE NOTICE '  -> Inserted 12 tags'; END $$;


-- ============================================================================
-- REGIONS (4 records)
-- Top-level geographic areas
-- ============================================================================

DO $$ BEGIN RAISE NOTICE 'Inserting REGIONS...'; END $$;

INSERT INTO regions (id, name, code, country_code, timezone) VALUES
('a1000000-0000-0000-0000-000000000001'::uuid, 'Western Region', 'WEST', 'US', 'America/Los_Angeles'),
('a1000000-0000-0000-0000-000000000002'::uuid, 'Central Region', 'CENTRAL', 'US', 'America/Chicago'),
('a1000000-0000-0000-0000-000000000003'::uuid, 'Eastern Region', 'EAST', 'US', 'America/New_York'),
('a1000000-0000-0000-0000-000000000004'::uuid, 'Southern Region', 'SOUTH', 'US', 'America/Chicago');

DO $$ BEGIN RAISE NOTICE '  -> Inserted 4 regions (Western, Central, Eastern, Southern)'; END $$;


-- ============================================================================
-- TERRITORIES (10 records)
-- Assignable sales territories within regions
-- ============================================================================

DO $$ BEGIN RAISE NOTICE 'Inserting TERRITORIES...'; END $$;

INSERT INTO territories (id, region_id, name, code, description, center_latitude, center_longitude, radius_miles, is_active) VALUES
-- Western Region (3 territories)
('b1000000-0000-0000-0000-000000000001'::uuid, 'a1000000-0000-0000-0000-000000000001'::uuid,
 'San Francisco Metro', 'SF-METRO', 'San Francisco Bay Area territory',
 37.7749, -122.4194, 25.0, true),
('b1000000-0000-0000-0000-000000000002'::uuid, 'a1000000-0000-0000-0000-000000000001'::uuid,
 'Los Angeles Downtown', 'LA-DT', 'Downtown Los Angeles territory',
 34.0522, -118.2437, 15.0, true),
('b1000000-0000-0000-0000-000000000003'::uuid, 'a1000000-0000-0000-0000-000000000001'::uuid,
 'Seattle Metro', 'SEA-METRO', 'Greater Seattle area',
 47.6062, -122.3321, 20.0, true),

-- Central Region (3 territories)
('b1000000-0000-0000-0000-000000000004'::uuid, 'a1000000-0000-0000-0000-000000000002'::uuid,
 'Denver Metro', 'DEN-METRO', 'Denver metropolitan area',
 39.7392, -104.9903, 20.0, true),
('b1000000-0000-0000-0000-000000000005'::uuid, 'a1000000-0000-0000-0000-000000000002'::uuid,
 'Chicago Loop', 'CHI-LOOP', 'Chicago downtown and surrounding areas',
 41.8781, -87.6298, 15.0, true),
('b1000000-0000-0000-0000-000000000006'::uuid, 'a1000000-0000-0000-0000-000000000002'::uuid,
 'Austin Metro', 'AUS-METRO', 'Austin Texas metropolitan area',
 30.2672, -97.7431, 18.0, true),

-- Eastern Region (2 territories)
('b1000000-0000-0000-0000-000000000007'::uuid, 'a1000000-0000-0000-0000-000000000003'::uuid,
 'New York City', 'NYC', 'New York City five boroughs',
 40.7128, -74.0060, 15.0, true),
('b1000000-0000-0000-0000-000000000008'::uuid, 'a1000000-0000-0000-0000-000000000003'::uuid,
 'Boston Metro', 'BOS-METRO', 'Greater Boston area',
 42.3601, -71.0589, 18.0, true),

-- Southern Region (2 territories)
('b1000000-0000-0000-0000-000000000009'::uuid, 'a1000000-0000-0000-0000-000000000004'::uuid,
 'Atlanta Metro', 'ATL-METRO', 'Atlanta metropolitan area',
 33.7490, -84.3880, 22.0, true),
('b1000000-0000-0000-0000-000000000010'::uuid, 'a1000000-0000-0000-0000-000000000004'::uuid,
 'Miami Metro', 'MIA-METRO', 'Miami-Dade area',
 25.7617, -80.1918, 20.0, true);

DO $$ BEGIN RAISE NOTICE '  -> Inserted 10 territories (3 West, 3 Central, 2 East, 2 South)'; END $$;


-- ============================================================================
-- FRANCHISEES (8 records)
-- Franchise owners/operators with company type classification
-- ============================================================================

DO $$ BEGIN RAISE NOTICE 'Inserting FRANCHISEES...'; END $$;

INSERT INTO franchisees (id, business_name, dba_name, franchise_number, owner_first_name, owner_last_name,
                         email, phone, address_line1, city, state_province, postal_code,
                         latitude, longitude, start_date, is_active, company_type_id) VALUES
-- Food Service Franchises (4 records)
('c1000000-0000-0000-0000-000000000001'::uuid,
 'Rocky Mountain Catering LLC', 'Mountain Fresh Catering', 'FRA-001',
 'John', 'Smith', 'john.smith@rmcatering.com', '(303) 555-0101',
 '1600 California St', 'Denver', 'CO', '80202',
 39.7456, -104.9885, '2022-01-15', true, 'c7000000-0000-0000-0000-000000000001'::uuid),

('c1000000-0000-0000-0000-000000000002'::uuid,
 'Bay Area Gourmet Inc', 'SF Gourmet Catering', 'FRA-002',
 'Sarah', 'Johnson', 'sarah@sfgourmet.com', '(415) 555-0202',
 '555 Market St', 'San Francisco', 'CA', '94105',
 37.7909, -122.3998, '2021-06-01', true, 'c7000000-0000-0000-0000-000000000001'::uuid),

('c1000000-0000-0000-0000-000000000003'::uuid,
 'Windy City Eats LLC', 'Chicago Corporate Catering', 'FRA-003',
 'Michael', 'Davis', 'mdavis@windycityeats.com', '(312) 555-0303',
 '233 S Wacker Dr', 'Chicago', 'IL', '60606',
 41.8789, -87.6359, '2020-09-01', true, 'c7000000-0000-0000-0000-000000000001'::uuid),

-- Default franchisee (Pittsburgh Catering) - used by app
('c2c5af5a-53a5-4d28-8218-3675c0942ead'::uuid,
 'Pittsburgh Catering Co', '', 'FRA-004',
 'Mike', '', '', '',
 '1687 Washington Road', 'Pittsburgh', 'PA', '15228',
 40.3732, -80.0432, CURRENT_DATE, true, 'c7000000-0000-0000-0000-000000000001'::uuid),

-- Manufacturing Company Examples (2 records)
('c1000000-0000-0000-0000-000000000010'::uuid,
 'Precision Parts Manufacturing', 'PPM Industries', 'MFG-001',
 'Robert', 'Chen', 'rchen@ppmind.com', '(614) 555-1010',
 '4500 Industrial Pkwy', 'Columbus', 'OH', '43228',
 39.9612, -83.0007, '2019-03-15', true, 'c7000000-0000-0000-0000-000000000010'::uuid),

('c1000000-0000-0000-0000-000000000011'::uuid,
 'TechComp Electronics', 'TechComp', 'MFG-002',
 'Jennifer', 'Park', 'jpark@techcomp.com', '(408) 555-1111',
 '2800 Technology Dr', 'San Jose', 'CA', '95134',
 37.3861, -121.9389, '2020-07-01', true, 'c7000000-0000-0000-0000-000000000012'::uuid),

-- Real Estate Company Examples (2 records)
('c1000000-0000-0000-0000-000000000020'::uuid,
 'Metro Commercial Realty', 'Metro Realty', 'RE-001',
 'David', 'Thompson', 'dthompson@metrorealty.com', '(212) 555-2020',
 '350 Fifth Avenue', 'New York', 'NY', '10118',
 40.7484, -73.9857, '2018-11-01', true, 'c7000000-0000-0000-0000-000000000020'::uuid),

('c1000000-0000-0000-0000-000000000021'::uuid,
 'Sunbelt Property Management', 'Sunbelt PM', 'RE-002',
 'Lisa', 'Rodriguez', 'lrodriguez@sunbeltpm.com', '(480) 555-2121',
 '3030 N Central Ave', 'Phoenix', 'AZ', '85012',
 33.4842, -112.0740, '2021-02-15', true, 'c7000000-0000-0000-0000-000000000022'::uuid);

DO $$ BEGIN RAISE NOTICE '  -> Inserted 8 franchisees (4 food service, 2 manufacturing, 2 real estate)'; END $$;


-- ============================================================================
-- FRANCHISEE TERRITORY ASSIGNMENTS (4 records)
-- ============================================================================

DO $$ BEGIN RAISE NOTICE 'Inserting FRANCHISEE_TERRITORIES...'; END $$;

INSERT INTO franchisee_territories (franchisee_id, territory_id, is_primary, assigned_date) VALUES
('c1000000-0000-0000-0000-000000000001'::uuid, 'b1000000-0000-0000-0000-000000000004'::uuid, true, '2022-01-15'),
('c1000000-0000-0000-0000-000000000002'::uuid, 'b1000000-0000-0000-0000-000000000001'::uuid, true, '2021-06-01'),
('c1000000-0000-0000-0000-000000000003'::uuid, 'b1000000-0000-0000-0000-000000000005'::uuid, true, '2020-09-01'),
('c1000000-0000-0000-0000-000000000020'::uuid, 'b1000000-0000-0000-0000-000000000007'::uuid, true, '2018-11-01');

DO $$ BEGIN RAISE NOTICE '  -> Inserted 4 franchisee-territory assignments'; END $$;


-- ============================================================================
-- STORE LOCATIONS (4 records)
-- Physical store locations for franchise operations
-- ============================================================================

DO $$ BEGIN RAISE NOTICE 'Inserting STORE_LOCATIONS...'; END $$;

INSERT INTO store_locations (id, franchisee_id, store_name, store_code, address_line1, city, state_province,
                             postal_code, latitude, longitude, geocode_source, geocoded_at,
                             default_search_radius_miles, phone, is_active, is_primary) VALUES
('d1000000-0000-0000-0000-000000000001'::uuid, 'c1000000-0000-0000-0000-000000000001'::uuid,
 'Downtown Denver Store', 'DEN-001', '1600 California St', 'Denver', 'CO', '80202',
 39.7456, -104.9885, 'nominatim', CURRENT_TIMESTAMP, 10.0, '(303) 555-0101', true, true),

('d1000000-0000-0000-0000-000000000002'::uuid, 'c1000000-0000-0000-0000-000000000002'::uuid,
 'Financial District Store', 'SF-001', '555 Market St', 'San Francisco', 'CA', '94105',
 37.7909, -122.3998, 'nominatim', CURRENT_TIMESTAMP, 8.0, '(415) 555-0202', true, true),

('d1000000-0000-0000-0000-000000000003'::uuid, 'c1000000-0000-0000-0000-000000000003'::uuid,
 'Willis Tower Store', 'CHI-001', '233 S Wacker Dr', 'Chicago', 'IL', '60606',
 41.8789, -87.6359, 'nominatim', CURRENT_TIMESTAMP, 12.0, '(312) 555-0303', true, true),

-- Default store location (Pittsburgh) - used by app
('c14a9f57-2ed2-4e30-9834-98614465ddbb'::uuid, 'c2c5af5a-53a5-4d28-8218-3675c0942ead'::uuid,
 'Pittsburgh Store', 'PIT-001', '1687 Washington Road', 'Pittsburgh', 'PA', '15228',
 40.3732, -80.0432, 'nominatim', CURRENT_TIMESTAMP, 25.0, '', true, true);

DO $$ BEGIN RAISE NOTICE '  -> Inserted 4 store locations'; END $$;


-- ============================================================================
-- SAMPLE PROSPECTS (7 records)
-- Example prospect records with AI scoring data
-- ============================================================================

DO $$ BEGIN RAISE NOTICE 'Inserting PROSPECTS with AI scoring data...'; END $$;

INSERT INTO prospects (id, territory_id, franchisee_id, business_name, business_type,
                       employee_count, employee_count_range, address_line1, city, state_province,
                       postal_code, latitude, longitude, primary_phone, website,
                       status, data_source, is_verified,
                       ai_score, optimized_score, relevance_score, ai_summary,
                       key_highlights, recommended_actions, data_sources) VALUES
-- Denver Prospects (3 records)
('e1000000-0000-0000-0000-000000000001'::uuid,
 'b1000000-0000-0000-0000-000000000004'::uuid, 'c1000000-0000-0000-0000-000000000001'::uuid,
 'TechStart Colorado', 'Technology Company', 150, 'medium',
 '1801 California St', 'Denver', 'CO', '80202',
 39.7489, -104.9878, '(303) 555-1001', 'https://techstartco.example.com', 'new', 'openstreetmap', true,
 78, 82, 0.7800,
 'Growing tech company with strong catering potential. Regular team meetings and client events.',
 'Growing technology startup|Regular team meetings|Downtown location|Active hiring',
 'Schedule initial consultation call|Send catering menu samples|Propose recurring lunch program',
 'OpenStreetMap,GooglePlaces'),

('e1000000-0000-0000-0000-000000000002'::uuid,
 'b1000000-0000-0000-0000-000000000004'::uuid, 'c1000000-0000-0000-0000-000000000001'::uuid,
 'Colorado Healthcare Partners', 'Healthcare Facility', 320, 'large',
 '1635 Aurora Ct', 'Denver', 'CO', '80045',
 39.7456, -104.8372, '(303) 555-1002', 'https://cohealthpartners.example.com', 'contacted', 'openstreetmap', true,
 88, 91, 0.8800,
 'Large healthcare facility with multiple departments. High volume catering needs for staff and events.',
 'Large employee base|Multiple departments|24/7 operations|Regular staff events',
 'Follow up on initial contact|Present enterprise catering options|Discuss dietary accommodations',
 'OpenStreetMap,BBB'),

('e1000000-0000-0000-0000-000000000003'::uuid,
 'b1000000-0000-0000-0000-000000000004'::uuid, 'c1000000-0000-0000-0000-000000000001'::uuid,
 'Mile High Law Group', 'Law Firm', 85, 'medium',
 '1700 Broadway', 'Denver', 'CO', '80290',
 39.7436, -104.9872, '(303) 555-1003', 'https://milehighlaw.example.com', 'qualified', 'openstreetmap', true,
 72, 78, 0.7200,
 'Professional law firm with regular client meetings. Premium catering opportunities.',
 'Premium clientele|Regular client meetings|Conference room facilities|Professional setting',
 'Send premium menu options|Propose client meeting packages|Schedule tasting session',
 'OpenStreetMap'),

-- San Francisco Prospects (2 records)
('e1000000-0000-0000-0000-000000000004'::uuid,
 'b1000000-0000-0000-0000-000000000001'::uuid, 'c1000000-0000-0000-0000-000000000002'::uuid,
 'Bay Innovations Inc', 'Technology Company', 280, 'large',
 '101 California St', 'San Francisco', 'CA', '94111',
 37.7929, -122.3984, '(415) 555-2001', 'https://bayinnovations.example.com', 'new', 'openstreetmap', true,
 85, 88, 0.8500,
 'Major tech company in financial district. High-volume daily catering potential.',
 'Large employee count|Tech-focused culture|Daily meal programs|Event hosting',
 'Research current catering provider|Prepare competitive proposal|Highlight tech-friendly ordering',
 'OpenStreetMap,GooglePlaces'),

('e1000000-0000-0000-0000-000000000005'::uuid,
 'b1000000-0000-0000-0000-000000000001'::uuid, 'c1000000-0000-0000-0000-000000000002'::uuid,
 'Pacific Financial Advisors', 'Financial Services', 120, 'medium',
 '425 Market St', 'San Francisco', 'CA', '94105',
 37.7912, -122.3985, '(415) 555-2002', 'https://pacificfa.example.com', 'proposal_sent', 'openstreetmap', true,
 70, 75, 0.7000,
 'Financial services firm with client-facing operations. Executive dining needs.',
 'Client-facing office|Executive team|Regular presentations|Professional atmosphere',
 'Follow up on proposal|Address any questions|Propose executive lunch program',
 'OpenStreetMap,BBB'),

-- Chicago Prospects (2 records)
('e1000000-0000-0000-0000-000000000006'::uuid,
 'b1000000-0000-0000-0000-000000000005'::uuid, 'c1000000-0000-0000-0000-000000000003'::uuid,
 'Midwest Corporate Holdings', 'Corporate Office', 450, 'large',
 '311 S Wacker Dr', 'Chicago', 'IL', '60606',
 41.8776, -87.6363, '(312) 555-3001', 'https://midwestcorp.example.com', 'new', 'openstreetmap', true,
 92, 94, 0.9200,
 'Large corporate headquarters with excellent catering potential. Multiple event spaces.',
 'Large corporate HQ|450+ employees|Multiple floors|Event facilities',
 'Schedule facility tour|Present corporate catering program|Discuss recurring service agreement',
 'OpenStreetMap,GooglePlaces,BBB'),

('e1000000-0000-0000-0000-000000000007'::uuid,
 'b1000000-0000-0000-0000-000000000005'::uuid, 'c1000000-0000-0000-0000-000000000003'::uuid,
 'Chicago Medical Center', 'Healthcare Facility', 890, 'enterprise',
 '710 N Lake Shore Dr', 'Chicago', 'IL', '60611',
 41.8953, -87.6171, '(312) 555-3002', 'https://chicagomedical.example.com', 'qualified', 'openstreetmap', true,
 94, 96, 0.9400,
 'Major medical center with extensive catering needs. High-volume potential across departments.',
 'Enterprise scale|24/7 operations|Multiple cafeterias|Event hosting|Staff appreciation',
 'Present enterprise solution|Discuss volume pricing|Propose pilot program for one department',
 'OpenStreetMap,GooglePlaces');

DO $$ BEGIN RAISE NOTICE '  -> Inserted 7 prospects (3 Denver, 2 SF, 2 Chicago) with AI scores'; END $$;


-- ============================================================================
-- PROSPECT SCORES (7 records)
-- AI and manual lead scoring
-- ============================================================================

DO $$ BEGIN RAISE NOTICE 'Inserting PROSPECT_SCORES...'; END $$;

INSERT INTO prospect_scores (prospect_id, total_score, score_grade, fit_score, engagement_score,
                            catering_potential_score, proximity_score, score_source, model_version, confidence) VALUES
('e1000000-0000-0000-0000-000000000001'::uuid, 82, 'B', 85, 75, 88, 90, 'ai_model', '1.0.0', 0.85),
('e1000000-0000-0000-0000-000000000002'::uuid, 91, 'A', 95, 88, 92, 85, 'ai_model', '1.0.0', 0.92),
('e1000000-0000-0000-0000-000000000003'::uuid, 78, 'B', 80, 72, 82, 88, 'ai_model', '1.0.0', 0.78),
('e1000000-0000-0000-0000-000000000004'::uuid, 88, 'B', 90, 82, 90, 85, 'ai_model', '1.0.0', 0.88),
('e1000000-0000-0000-0000-000000000005'::uuid, 75, 'B', 78, 70, 80, 82, 'ai_model', '1.0.0', 0.75),
('e1000000-0000-0000-0000-000000000006'::uuid, 94, 'A', 96, 90, 95, 92, 'ai_model', '1.0.0', 0.94),
('e1000000-0000-0000-0000-000000000007'::uuid, 96, 'A', 98, 92, 98, 88, 'ai_model', '1.0.0', 0.96);

DO $$ BEGIN RAISE NOTICE '  -> Inserted 7 prospect scores (3 grade A, 4 grade B)'; END $$;


-- ============================================================================
-- TERRITORY DEMOGRAPHICS (3 records)
-- Demographic and market data per territory
-- ============================================================================

DO $$ BEGIN RAISE NOTICE 'Inserting TERRITORY_DEMOGRAPHICS...'; END $$;

INSERT INTO territory_demographics (territory_id, total_population, population_density, median_age,
                                   total_households, median_household_income, total_businesses,
                                   total_employees, business_density, data_year, data_source) VALUES
('b1000000-0000-0000-0000-000000000004'::uuid, 716492, 4521.3, 34.5, 305678, 85432.00,
 42350, 528900, 267.8, 2024, 'US Census Bureau'),
('b1000000-0000-0000-0000-000000000001'::uuid, 873965, 18634.2, 38.2, 352890, 126750.00,
 68420, 892340, 1458.6, 2024, 'US Census Bureau'),
('b1000000-0000-0000-0000-000000000005'::uuid, 2693976, 11841.8, 35.8, 1087650, 78650.00,
 125680, 1892450, 552.4, 2024, 'US Census Bureau');

DO $$ BEGIN RAISE NOTICE '  -> Inserted 3 territory demographics (Denver, SF, Chicago)'; END $$;


-- ============================================================================
-- USERS (4 records)
-- Application users with authentication
-- NOTE: Password hashes are for development only (MD5)
-- In production, use bcrypt or Argon2 with proper salting
-- ============================================================================

DO $$ BEGIN RAISE NOTICE 'Inserting USERS...'; END $$;

INSERT INTO users (id, email, password_hash, first_name, last_name, role, franchisee_id, is_active, is_verified) VALUES
-- Admin user (password: admin123)
('f1000000-0000-0000-0000-000000000001'::uuid,
 'admin@franchiseai.com',
 '240be518fabd2724ddb6f04eeb9d5b59',  -- MD5 hash of 'admin123' (dev only)
 'System', 'Administrator', 'admin', NULL, true, true),

-- Pittsburgh franchisee user (password: mike123)
('f1000000-0000-0000-0000-000000000002'::uuid,
 'mike@pittsburghcatering.com',
 'e99a18c428cb38d5f260853678922e03',  -- MD5 hash of 'mike123' (dev only)
 'Mike', 'Owner', 'franchisee',
 'c2c5af5a-53a5-4d28-8218-3675c0942ead'::uuid, true, true),

-- Manufacturing company user (password: robert123)
('f1000000-0000-0000-0000-000000000003'::uuid,
 'rchen@ppmind.com',
 '7c6a180b36896a65c3d4e5f9a5f2a5e5',  -- MD5 hash (dev only)
 'Robert', 'Chen', 'franchisee',
 'c1000000-0000-0000-0000-000000000010'::uuid, true, true),

-- Real estate company user (password: david123)
('f1000000-0000-0000-0000-000000000004'::uuid,
 'dthompson@metrorealty.com',
 '3c59dc048e8850243be8079a5c74d079',  -- MD5 hash (dev only)
 'David', 'Thompson', 'franchisee',
 'c1000000-0000-0000-0000-000000000020'::uuid, true, true);

DO $$ BEGIN RAISE NOTICE '  -> Inserted 4 users (1 admin, 3 franchisees)'; END $$;


-- ============================================================================
-- SCORING RULES (13 records)
-- Configurable rules for prospect score adjustments
-- ============================================================================

DO $$ BEGIN RAISE NOTICE 'Inserting SCORING_RULES...'; END $$;

-- Penalty rules (5 records - negative adjustments for missing/incomplete data)
INSERT INTO scoring_rules (rule_id, name, description, is_penalty, enabled, default_points, current_points, min_points, max_points) VALUES
('no_address', 'Missing Address', 'Prospects without addresses are harder to contact and verify', true, true, -10, -10, -25, 0),
('no_employees', 'Missing Employee Count', 'Unknown employee count makes potential harder to estimate', true, true, -3, -3, -15, 0),
('no_contact', 'Missing Contact Info', 'No phone or email makes outreach difficult', true, true, -5, -5, -20, 0),
('no_website', 'Missing Website', 'No web presence may indicate smaller operation', true, true, -2, -2, -10, 0),
('incomplete_profile', 'Incomplete Business Profile', 'Missing key business information', true, true, -4, -4, -15, 0);

DO $$ BEGIN RAISE NOTICE '  -> Inserted 5 penalty rules'; END $$;

-- Bonus rules (8 records - positive adjustments for quality indicators)
INSERT INTO scoring_rules (rule_id, name, description, is_penalty, enabled, default_points, current_points, min_points, max_points) VALUES
('verified', 'Verified Business', 'Business has been verified through data sources', false, true, 5, 5, 0, 15),
('bbb_accredited', 'BBB Accredited', 'Business is accredited by the Better Business Bureau', false, true, 10, 10, 0, 20),
('high_rating', 'High Google Rating', 'Business has 4.5+ star Google rating', false, true, 5, 5, 0, 15),
('conference_room', 'Has Conference Room', 'Business has conference facilities', false, true, 5, 5, 0, 15),
('event_space', 'Has Event Space', 'Business has dedicated event space', false, true, 7, 7, 0, 20),
('large_company', 'Large Company (100+ employees)', 'Larger companies have more opportunities', false, true, 8, 8, 0, 20),
('growth_indicators', 'Growth Indicators', 'Business shows signs of growth or expansion', false, true, 6, 6, 0, 18),
('multiple_locations', 'Multiple Locations', 'Business has multiple locations', false, true, 10, 10, 0, 25);

DO $$ BEGIN RAISE NOTICE '  -> Inserted 8 bonus rules'; END $$;


-- ============================================================================
-- SEED DATA COMPLETE
-- ============================================================================

DO $$ BEGIN RAISE NOTICE ''; END $$;
DO $$ BEGIN RAISE NOTICE '=== FranchiseAI Seed Data Complete ==='; END $$;
DO $$ BEGIN RAISE NOTICE 'Summary:'; END $$;
DO $$ BEGIN RAISE NOTICE '  - 9 company types'; END $$;
DO $$ BEGIN RAISE NOTICE '  - 16 app config settings'; END $$;
DO $$ BEGIN RAISE NOTICE '  - 15 industries'; END $$;
DO $$ BEGIN RAISE NOTICE '  - 12 tags'; END $$;
DO $$ BEGIN RAISE NOTICE '  - 4 regions'; END $$;
DO $$ BEGIN RAISE NOTICE '  - 10 territories'; END $$;
DO $$ BEGIN RAISE NOTICE '  - 8 franchisees'; END $$;
DO $$ BEGIN RAISE NOTICE '  - 4 franchisee-territory assignments'; END $$;
DO $$ BEGIN RAISE NOTICE '  - 4 store locations'; END $$;
DO $$ BEGIN RAISE NOTICE '  - 7 prospects with AI scoring data'; END $$;
DO $$ BEGIN RAISE NOTICE '  - 7 prospect scores'; END $$;
DO $$ BEGIN RAISE NOTICE '  - 3 territory demographics'; END $$;
DO $$ BEGIN RAISE NOTICE '  - 4 users'; END $$;
DO $$ BEGIN RAISE NOTICE '  - 13 scoring rules (5 penalties, 8 bonuses)'; END $$;
DO $$ BEGIN RAISE NOTICE ''; END $$;
DO $$ BEGIN RAISE NOTICE 'Default login: mike@pittsburghcatering.com / mike123'; END $$;
DO $$ BEGIN RAISE NOTICE '=== END ==='; END $$;
