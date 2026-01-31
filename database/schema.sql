-- ============================================================================
-- FranchiseAI Prospect Discovery Platform - PostgreSQL Schema
-- ApiLogicServer Data Model for Prospect Tracking
-- ============================================================================

-- Enable required extensions (no PostGIS needed)
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";

-- ============================================================================
-- ENUM TYPES
-- ============================================================================

CREATE TYPE prospect_status AS ENUM (
    'new',
    'contacted',
    'qualified',
    'proposal_sent',
    'negotiating',
    'won',
    'lost',
    'inactive'
);

CREATE TYPE business_size AS ENUM (
    'micro',        -- 1-9 employees
    'small',        -- 10-49 employees
    'medium',       -- 50-249 employees
    'large',        -- 250+ employees
    'enterprise'    -- 1000+ employees
);

CREATE TYPE outreach_type AS ENUM (
    'email',
    'phone',
    'in_person',
    'linkedin',
    'mail',
    'referral',
    'event'
);

CREATE TYPE score_source AS ENUM (
    'ai_model',
    'manual',
    'rule_based',
    'imported'
);

-- ============================================================================
-- CORE TABLES
-- ============================================================================

-- ---------------------------------------------------------------------------
-- Regions: Top-level geographic areas
-- ---------------------------------------------------------------------------
CREATE TABLE regions (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    name VARCHAR(100) NOT NULL,
    code VARCHAR(20) UNIQUE NOT NULL,
    country_code CHAR(2) NOT NULL DEFAULT 'US',
    timezone VARCHAR(50) DEFAULT 'America/New_York',

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

COMMENT ON TABLE regions IS 'Top-level geographic regions for franchise organization';

-- ---------------------------------------------------------------------------
-- Territories: Assignable sales territories within regions
-- ---------------------------------------------------------------------------
CREATE TABLE territories (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    region_id UUID NOT NULL REFERENCES regions(id) ON DELETE CASCADE,

    name VARCHAR(100) NOT NULL,
    code VARCHAR(30) UNIQUE NOT NULL,
    description TEXT,

    -- Center point (simple lat/lng)
    center_latitude DECIMAL(10, 8),
    center_longitude DECIMAL(11, 8),

    -- Territory metadata
    radius_miles DECIMAL(10, 2),
    zip_codes TEXT[],  -- Array of ZIP codes in territory

    -- Status
    is_active BOOLEAN DEFAULT true,

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_territories_region ON territories(region_id);

COMMENT ON TABLE territories IS 'Sales territories with geographic boundaries';

-- ---------------------------------------------------------------------------
-- Territory Demographics: Demographic data for each territory
-- ---------------------------------------------------------------------------
CREATE TABLE territory_demographics (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    territory_id UUID NOT NULL REFERENCES territories(id) ON DELETE CASCADE,

    -- Population data
    total_population INTEGER,
    population_density DECIMAL(10, 2),  -- per square mile
    median_age DECIMAL(4, 1),

    -- Household data
    total_households INTEGER,
    median_household_income DECIMAL(12, 2),
    average_household_size DECIMAL(3, 1),

    -- Income distribution
    income_under_25k_pct DECIMAL(5, 2),
    income_25k_50k_pct DECIMAL(5, 2),
    income_50k_75k_pct DECIMAL(5, 2),
    income_75k_100k_pct DECIMAL(5, 2),
    income_100k_150k_pct DECIMAL(5, 2),
    income_over_150k_pct DECIMAL(5, 2),

    -- Business data
    total_businesses INTEGER,
    total_employees INTEGER,
    business_density DECIMAL(10, 2),  -- businesses per square mile

    -- Market indicators
    daytime_population INTEGER,
    commuter_inflow INTEGER,
    commuter_outflow INTEGER,

    -- Data freshness
    data_year INTEGER NOT NULL,
    data_source VARCHAR(100),

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,

    UNIQUE(territory_id, data_year)
);

CREATE INDEX idx_territory_demographics_territory ON territory_demographics(territory_id);

COMMENT ON TABLE territory_demographics IS 'Demographic and market data per territory';

-- ---------------------------------------------------------------------------
-- Franchisees: Franchise owners/operators
-- ---------------------------------------------------------------------------
CREATE TABLE franchisees (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),

    -- Business info
    business_name VARCHAR(200) NOT NULL,
    dba_name VARCHAR(200),
    franchise_number VARCHAR(50) UNIQUE,

    -- Contact info
    owner_first_name VARCHAR(100),
    owner_last_name VARCHAR(100),
    email VARCHAR(255),
    phone VARCHAR(30),

    -- Address
    address_line1 VARCHAR(200),
    address_line2 VARCHAR(100),
    city VARCHAR(100),
    state_province VARCHAR(50),
    postal_code VARCHAR(20),
    country_code CHAR(2) DEFAULT 'US',

    -- Geolocation (simple lat/lng)
    latitude DECIMAL(10, 8),
    longitude DECIMAL(11, 8),

    -- Franchise details
    start_date DATE,
    contract_end_date DATE,
    is_active BOOLEAN DEFAULT true,

    -- Performance metrics
    monthly_revenue_target DECIMAL(12, 2),
    ytd_revenue DECIMAL(14, 2),

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_franchisees_active ON franchisees(is_active) WHERE is_active = true;

COMMENT ON TABLE franchisees IS 'Franchise owners and their business details';

-- ---------------------------------------------------------------------------
-- Franchisee Territories: Many-to-many relationship
-- ---------------------------------------------------------------------------
CREATE TABLE franchisee_territories (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    franchisee_id UUID NOT NULL REFERENCES franchisees(id) ON DELETE CASCADE,
    territory_id UUID NOT NULL REFERENCES territories(id) ON DELETE CASCADE,

    is_primary BOOLEAN DEFAULT false,
    assigned_date DATE DEFAULT CURRENT_DATE,

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,

    UNIQUE(franchisee_id, territory_id)
);

CREATE INDEX idx_franchisee_territories_franchisee ON franchisee_territories(franchisee_id);
CREATE INDEX idx_franchisee_territories_territory ON franchisee_territories(territory_id);

COMMENT ON TABLE franchisee_territories IS 'Assignment of territories to franchisees';

-- ---------------------------------------------------------------------------
-- Industries: Industry classification for prospects
-- ---------------------------------------------------------------------------
CREATE TABLE industries (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),

    name VARCHAR(100) NOT NULL,
    naics_code VARCHAR(10),
    sic_code VARCHAR(10),

    -- Catering relevance
    catering_potential_score INTEGER CHECK (catering_potential_score BETWEEN 1 AND 10),
    typical_order_size VARCHAR(50),
    peak_seasons TEXT[],

    parent_id UUID REFERENCES industries(id),

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_industries_naics ON industries(naics_code);
CREATE INDEX idx_industries_parent ON industries(parent_id);

COMMENT ON TABLE industries IS 'Industry classification with catering relevance scoring';

-- ---------------------------------------------------------------------------
-- Prospects: Prospective catering clients
-- ---------------------------------------------------------------------------
CREATE TABLE prospects (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),

    -- Assignment
    territory_id UUID REFERENCES territories(id),
    franchisee_id UUID REFERENCES franchisees(id),
    assigned_to_user_id UUID,  -- Sales rep user ID

    -- Business identification
    business_name VARCHAR(300) NOT NULL,
    dba_name VARCHAR(300),
    legal_name VARCHAR(300),

    -- Industry
    industry_id UUID REFERENCES industries(id),
    industry_naics VARCHAR(10),
    business_type VARCHAR(100),

    -- Business size
    employee_count INTEGER,
    employee_count_range business_size,
    annual_revenue DECIMAL(14, 2),
    year_established INTEGER,

    -- Address
    address_line1 VARCHAR(200),
    address_line2 VARCHAR(100),
    city VARCHAR(100),
    state_province VARCHAR(50),
    postal_code VARCHAR(20),
    country_code CHAR(2) DEFAULT 'US',

    -- Geolocation (simple lat/lng)
    latitude DECIMAL(10, 8),
    longitude DECIMAL(11, 8),
    geocode_accuracy VARCHAR(20),

    -- Contact
    primary_phone VARCHAR(30),
    secondary_phone VARCHAR(30),
    email VARCHAR(255),
    website VARCHAR(500),

    -- Social presence
    linkedin_url VARCHAR(500),
    facebook_url VARCHAR(500),

    -- Status tracking
    status prospect_status DEFAULT 'new',
    status_changed_at TIMESTAMP WITH TIME ZONE,

    -- Source tracking
    data_source VARCHAR(100),
    source_record_id VARCHAR(200),

    -- Flags
    is_verified BOOLEAN DEFAULT false,
    is_duplicate BOOLEAN DEFAULT false,
    duplicate_of_id UUID REFERENCES prospects(id),
    do_not_contact BOOLEAN DEFAULT false,

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_prospects_territory ON prospects(territory_id);
CREATE INDEX idx_prospects_franchisee ON prospects(franchisee_id);
CREATE INDEX idx_prospects_status ON prospects(status);
CREATE INDEX idx_prospects_industry ON prospects(industry_id);
CREATE INDEX idx_prospects_postal_code ON prospects(postal_code);
CREATE INDEX idx_prospects_created ON prospects(created_at DESC);
CREATE INDEX idx_prospects_business_name ON prospects USING gin(to_tsvector('english', business_name));

COMMENT ON TABLE prospects IS 'Prospective catering clients with full business details';

-- ---------------------------------------------------------------------------
-- Prospect Scores: AI and manual lead scoring
-- ---------------------------------------------------------------------------
CREATE TABLE prospect_scores (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    prospect_id UUID NOT NULL REFERENCES prospects(id) ON DELETE CASCADE,

    -- Overall score
    total_score INTEGER NOT NULL CHECK (total_score BETWEEN 0 AND 100),
    score_grade CHAR(1) CHECK (score_grade IN ('A', 'B', 'C', 'D', 'F')),

    -- Component scores (0-100 scale)
    fit_score INTEGER CHECK (fit_score BETWEEN 0 AND 100),
    engagement_score INTEGER CHECK (engagement_score BETWEEN 0 AND 100),
    intent_score INTEGER CHECK (intent_score BETWEEN 0 AND 100),

    -- Catering-specific scores
    catering_potential_score INTEGER CHECK (catering_potential_score BETWEEN 0 AND 100),
    order_frequency_score INTEGER CHECK (order_frequency_score BETWEEN 0 AND 100),
    budget_score INTEGER CHECK (budget_score BETWEEN 0 AND 100),

    -- Location scores
    proximity_score INTEGER CHECK (proximity_score BETWEEN 0 AND 100),
    market_density_score INTEGER CHECK (market_density_score BETWEEN 0 AND 100),

    -- Scoring metadata
    score_source score_source NOT NULL,
    model_version VARCHAR(50),
    confidence DECIMAL(5, 4),  -- 0.0000 to 1.0000

    -- Score factors (JSONB for flexibility)
    score_factors JSONB,

    scored_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    expires_at TIMESTAMP WITH TIME ZONE,

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_prospect_scores_prospect ON prospect_scores(prospect_id);
CREATE INDEX idx_prospect_scores_total ON prospect_scores(total_score DESC);
CREATE INDEX idx_prospect_scores_grade ON prospect_scores(score_grade);
CREATE INDEX idx_prospect_scores_scored_at ON prospect_scores(scored_at DESC);

COMMENT ON TABLE prospect_scores IS 'Lead scores from AI models and manual assessment';

-- ---------------------------------------------------------------------------
-- Prospect Contacts: Contact people at prospect businesses
-- ---------------------------------------------------------------------------
CREATE TABLE prospect_contacts (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    prospect_id UUID NOT NULL REFERENCES prospects(id) ON DELETE CASCADE,

    -- Name
    first_name VARCHAR(100),
    last_name VARCHAR(100),
    full_name VARCHAR(200),

    -- Role
    title VARCHAR(150),
    department VARCHAR(100),
    is_decision_maker BOOLEAN DEFAULT false,
    is_primary_contact BOOLEAN DEFAULT false,

    -- Contact info
    email VARCHAR(255),
    phone VARCHAR(30),
    phone_extension VARCHAR(10),
    mobile VARCHAR(30),
    linkedin_url VARCHAR(500),

    -- Preferences
    preferred_contact_method outreach_type,
    best_time_to_contact VARCHAR(100),

    -- Status
    is_active BOOLEAN DEFAULT true,

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_prospect_contacts_prospect ON prospect_contacts(prospect_id);
CREATE INDEX idx_prospect_contacts_email ON prospect_contacts(email);

COMMENT ON TABLE prospect_contacts IS 'Individual contacts at prospect businesses';

-- ---------------------------------------------------------------------------
-- Prospect Outreach: Track all outreach activities
-- ---------------------------------------------------------------------------
CREATE TABLE prospect_outreach (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    prospect_id UUID NOT NULL REFERENCES prospects(id) ON DELETE CASCADE,
    contact_id UUID REFERENCES prospect_contacts(id),
    franchisee_id UUID REFERENCES franchisees(id),
    user_id UUID,  -- Sales rep user ID

    -- Outreach details
    outreach_type outreach_type NOT NULL,
    outreach_date TIMESTAMP WITH TIME ZONE NOT NULL,

    -- Content
    subject VARCHAR(300),
    notes TEXT,

    -- Outcome
    was_successful BOOLEAN,
    response_received BOOLEAN DEFAULT false,
    response_date TIMESTAMP WITH TIME ZONE,
    follow_up_required BOOLEAN DEFAULT false,
    follow_up_date DATE,

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_prospect_outreach_prospect ON prospect_outreach(prospect_id);
CREATE INDEX idx_prospect_outreach_date ON prospect_outreach(outreach_date DESC);
CREATE INDEX idx_prospect_outreach_franchisee ON prospect_outreach(franchisee_id);

COMMENT ON TABLE prospect_outreach IS 'History of all outreach activities to prospects';

-- ---------------------------------------------------------------------------
-- Prospect Notes: General notes and activity log
-- ---------------------------------------------------------------------------
CREATE TABLE prospect_notes (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    prospect_id UUID NOT NULL REFERENCES prospects(id) ON DELETE CASCADE,
    user_id UUID,

    note_type VARCHAR(50) DEFAULT 'general',
    content TEXT NOT NULL,

    is_pinned BOOLEAN DEFAULT false,

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_prospect_notes_prospect ON prospect_notes(prospect_id);
CREATE INDEX idx_prospect_notes_created ON prospect_notes(created_at DESC);

COMMENT ON TABLE prospect_notes IS 'Notes and activity log for prospects';

-- ---------------------------------------------------------------------------
-- Prospect Tags: Flexible tagging system
-- ---------------------------------------------------------------------------
CREATE TABLE tags (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    name VARCHAR(50) NOT NULL UNIQUE,
    color VARCHAR(7),  -- Hex color code
    description VARCHAR(200),

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE prospect_tags (
    prospect_id UUID NOT NULL REFERENCES prospects(id) ON DELETE CASCADE,
    tag_id UUID NOT NULL REFERENCES tags(id) ON DELETE CASCADE,

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,

    PRIMARY KEY (prospect_id, tag_id)
);

CREATE INDEX idx_prospect_tags_tag ON prospect_tags(tag_id);

COMMENT ON TABLE tags IS 'Reusable tags for categorizing prospects';
COMMENT ON TABLE prospect_tags IS 'Many-to-many relationship between prospects and tags';

-- ---------------------------------------------------------------------------
-- Saved Searches: Store user search queries
-- ---------------------------------------------------------------------------
CREATE TABLE saved_searches (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    user_id UUID,
    franchisee_id UUID REFERENCES franchisees(id),

    name VARCHAR(100) NOT NULL,
    description TEXT,

    -- Search parameters (stored as JSONB)
    search_params JSONB NOT NULL,

    -- Usage tracking
    last_run_at TIMESTAMP WITH TIME ZONE,
    run_count INTEGER DEFAULT 0,

    is_shared BOOLEAN DEFAULT false,

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_saved_searches_franchisee ON saved_searches(franchisee_id);

COMMENT ON TABLE saved_searches IS 'Saved search queries for quick access';

-- ---------------------------------------------------------------------------
-- Data Source Sync: Track data imports from MCP services
-- ---------------------------------------------------------------------------
CREATE TABLE data_source_sync (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),

    source_name VARCHAR(100) NOT NULL,  -- e.g., 'openstreetmap', 'wikidata'
    sync_type VARCHAR(50) NOT NULL,     -- 'full', 'incremental'

    started_at TIMESTAMP WITH TIME ZONE NOT NULL,
    completed_at TIMESTAMP WITH TIME ZONE,

    records_fetched INTEGER DEFAULT 0,
    records_created INTEGER DEFAULT 0,
    records_updated INTEGER DEFAULT 0,
    records_failed INTEGER DEFAULT 0,

    status VARCHAR(20) DEFAULT 'running',
    error_message TEXT,

    sync_params JSONB,

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_data_source_sync_source ON data_source_sync(source_name);
CREATE INDEX idx_data_source_sync_started ON data_source_sync(started_at DESC);

COMMENT ON TABLE data_source_sync IS 'Track data synchronization from MCP data services';


-- ============================================================================
-- VIEWS
-- ============================================================================

-- ---------------------------------------------------------------------------
-- View: Prospects with latest score
-- ---------------------------------------------------------------------------
CREATE VIEW v_prospects_with_scores AS
SELECT
    p.*,
    ps.total_score,
    ps.score_grade,
    ps.fit_score,
    ps.catering_potential_score,
    ps.scored_at,
    ps.score_source,
    t.name as territory_name,
    f.business_name as franchisee_name,
    i.name as industry_name
FROM prospects p
LEFT JOIN LATERAL (
    SELECT * FROM prospect_scores
    WHERE prospect_id = p.id
    ORDER BY scored_at DESC
    LIMIT 1
) ps ON true
LEFT JOIN territories t ON p.territory_id = t.id
LEFT JOIN franchisees f ON p.franchisee_id = f.id
LEFT JOIN industries i ON p.industry_id = i.id;

COMMENT ON VIEW v_prospects_with_scores IS 'Prospects with their latest score and related data';

-- ---------------------------------------------------------------------------
-- View: Territory summary with prospect counts
-- ---------------------------------------------------------------------------
CREATE VIEW v_territory_summary AS
SELECT
    t.id,
    t.name,
    t.code,
    r.name as region_name,
    COUNT(DISTINCT p.id) as total_prospects,
    COUNT(DISTINCT p.id) FILTER (WHERE p.status = 'new') as new_prospects,
    COUNT(DISTINCT p.id) FILTER (WHERE p.status = 'qualified') as qualified_prospects,
    COUNT(DISTINCT p.id) FILTER (WHERE p.status = 'won') as won_prospects,
    COUNT(DISTINCT ft.franchisee_id) as assigned_franchisees,
    td.median_household_income,
    td.total_businesses,
    td.population_density
FROM territories t
LEFT JOIN regions r ON t.region_id = r.id
LEFT JOIN prospects p ON p.territory_id = t.id
LEFT JOIN franchisee_territories ft ON ft.territory_id = t.id
LEFT JOIN LATERAL (
    SELECT * FROM territory_demographics
    WHERE territory_id = t.id
    ORDER BY data_year DESC
    LIMIT 1
) td ON true
GROUP BY t.id, t.name, t.code, r.name,
         td.median_household_income, td.total_businesses, td.population_density;

COMMENT ON VIEW v_territory_summary IS 'Territory overview with prospect counts and demographics';

-- ---------------------------------------------------------------------------
-- View: Franchisee performance dashboard
-- ---------------------------------------------------------------------------
CREATE VIEW v_franchisee_dashboard AS
SELECT
    f.id,
    f.business_name,
    f.franchise_number,
    f.city,
    f.state_province,
    COUNT(DISTINCT p.id) as total_prospects,
    COUNT(DISTINCT p.id) FILTER (WHERE p.status = 'new') as new_leads,
    COUNT(DISTINCT p.id) FILTER (WHERE p.status IN ('contacted', 'qualified', 'proposal_sent', 'negotiating')) as active_pipeline,
    COUNT(DISTINCT p.id) FILTER (WHERE p.status = 'won') as won_deals,
    COUNT(DISTINCT po.id) FILTER (WHERE po.outreach_date > CURRENT_DATE - INTERVAL '30 days') as outreach_last_30_days,
    AVG(ps.total_score) as avg_prospect_score,
    COUNT(DISTINCT ft.territory_id) as territory_count
FROM franchisees f
LEFT JOIN prospects p ON p.franchisee_id = f.id
LEFT JOIN prospect_outreach po ON po.franchisee_id = f.id
LEFT JOIN prospect_scores ps ON ps.prospect_id = p.id
LEFT JOIN franchisee_territories ft ON ft.franchisee_id = f.id
WHERE f.is_active = true
GROUP BY f.id, f.business_name, f.franchise_number, f.city, f.state_province;

COMMENT ON VIEW v_franchisee_dashboard IS 'Franchisee performance metrics dashboard';

-- ---------------------------------------------------------------------------
-- View: Hot prospects (high score, recent activity)
-- ---------------------------------------------------------------------------
CREATE VIEW v_hot_prospects AS
SELECT
    p.id,
    p.business_name,
    p.city,
    p.state_province,
    p.status,
    p.employee_count,
    ps.total_score,
    ps.score_grade,
    ps.catering_potential_score,
    t.name as territory_name,
    f.business_name as franchisee_name,
    i.name as industry_name,
    (
        SELECT MAX(outreach_date)
        FROM prospect_outreach
        WHERE prospect_id = p.id
    ) as last_contact_date
FROM prospects p
INNER JOIN LATERAL (
    SELECT * FROM prospect_scores
    WHERE prospect_id = p.id
    ORDER BY scored_at DESC
    LIMIT 1
) ps ON ps.total_score >= 70
LEFT JOIN territories t ON p.territory_id = t.id
LEFT JOIN franchisees f ON p.franchisee_id = f.id
LEFT JOIN industries i ON p.industry_id = i.id
WHERE p.status NOT IN ('won', 'lost', 'inactive')
  AND p.do_not_contact = false
ORDER BY ps.total_score DESC, ps.scored_at DESC;

COMMENT ON VIEW v_hot_prospects IS 'High-scoring prospects ready for outreach';


-- ============================================================================
-- FUNCTIONS
-- ============================================================================

-- ---------------------------------------------------------------------------
-- Function: Update timestamp trigger
-- ---------------------------------------------------------------------------
CREATE OR REPLACE FUNCTION update_updated_at_column()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = CURRENT_TIMESTAMP;
    RETURN NEW;
END;
$$ language 'plpgsql';

-- Apply to all tables with updated_at
CREATE TRIGGER update_regions_updated_at BEFORE UPDATE ON regions
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_territories_updated_at BEFORE UPDATE ON territories
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_territory_demographics_updated_at BEFORE UPDATE ON territory_demographics
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_franchisees_updated_at BEFORE UPDATE ON franchisees
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_prospects_updated_at BEFORE UPDATE ON prospects
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_prospect_contacts_updated_at BEFORE UPDATE ON prospect_contacts
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_prospect_notes_updated_at BEFORE UPDATE ON prospect_notes
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

CREATE TRIGGER update_saved_searches_updated_at BEFORE UPDATE ON saved_searches
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

-- ---------------------------------------------------------------------------
-- Function: Calculate prospect score grade from total score
-- ---------------------------------------------------------------------------
CREATE OR REPLACE FUNCTION calculate_score_grade(total_score INTEGER)
RETURNS CHAR(1) AS $$
BEGIN
    RETURN CASE
        WHEN total_score >= 90 THEN 'A'
        WHEN total_score >= 75 THEN 'B'
        WHEN total_score >= 60 THEN 'C'
        WHEN total_score >= 40 THEN 'D'
        ELSE 'F'
    END;
END;
$$ LANGUAGE plpgsql IMMUTABLE;

COMMENT ON FUNCTION calculate_score_grade IS 'Convert numeric score to letter grade';


-- ============================================================================
-- APPLICATION CONFIGURATION TABLE
-- ============================================================================

-- ---------------------------------------------------------------------------
-- App Config: Stores application settings (business/feature settings only)
-- ---------------------------------------------------------------------------
-- NOTE: Infrastructure configs (ApiLogicServer URL, API keys, service endpoints)
-- are stored in local config/app_config.json, NOT in the database.
-- This table stores only business/feature settings managed through the app.

CREATE TABLE app_config (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),

    config_key VARCHAR(100) NOT NULL UNIQUE,
    config_value TEXT,
    config_type VARCHAR(20) DEFAULT 'string',  -- string, integer, boolean, json

    -- Categorization
    category VARCHAR(50) NOT NULL,  -- 'features', 'display', 'business'

    -- Metadata
    description TEXT,
    is_sensitive BOOLEAN DEFAULT false,
    is_required BOOLEAN DEFAULT false,
    default_value TEXT,

    -- Validation
    validation_regex VARCHAR(500),
    allowed_values TEXT[],  -- For enum-like configs

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_app_config_category ON app_config(category);
CREATE INDEX idx_app_config_key ON app_config(config_key);

CREATE TRIGGER update_app_config_updated_at BEFORE UPDATE ON app_config
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

COMMENT ON TABLE app_config IS 'Application configuration settings (business/feature settings only)';

-- ---------------------------------------------------------------------------
-- Store Locations: Franchise store locations for the app
-- ---------------------------------------------------------------------------
CREATE TABLE store_locations (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    franchisee_id UUID REFERENCES franchisees(id) ON DELETE CASCADE,

    -- Store identification
    store_name VARCHAR(200) NOT NULL,
    store_code VARCHAR(50) UNIQUE,

    -- Address
    address_line1 VARCHAR(200) NOT NULL,
    address_line2 VARCHAR(100),
    city VARCHAR(100) DEFAULT '',
    state_province VARCHAR(50) DEFAULT '',
    postal_code VARCHAR(20) DEFAULT '',
    country_code CHAR(2) DEFAULT 'US',

    -- Geolocation (simple lat/lng)
    latitude DECIMAL(10, 8),
    longitude DECIMAL(11, 8),
    geocode_source VARCHAR(50),  -- 'nominatim', 'google', 'manual'
    geocoded_at TIMESTAMP WITH TIME ZONE,

    -- Search preferences
    default_search_radius_miles DECIMAL(6, 2) DEFAULT 5.0,

    -- Contact
    phone VARCHAR(30),
    email VARCHAR(255),

    -- Status
    is_active BOOLEAN DEFAULT true,
    is_primary BOOLEAN DEFAULT false,

    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_store_locations_franchisee ON store_locations(franchisee_id);
CREATE INDEX idx_store_locations_active ON store_locations(is_active) WHERE is_active = true;

CREATE TRIGGER update_store_locations_updated_at BEFORE UPDATE ON store_locations
    FOR EACH ROW EXECUTE FUNCTION update_updated_at_column();

-- Migration: Make city/state/postal nullable for existing tables
-- (These fields may not always be available from geocoding)
ALTER TABLE store_locations ALTER COLUMN city SET DEFAULT '';
ALTER TABLE store_locations ALTER COLUMN city DROP NOT NULL;
ALTER TABLE store_locations ALTER COLUMN state_province SET DEFAULT '';
ALTER TABLE store_locations ALTER COLUMN state_province DROP NOT NULL;
ALTER TABLE store_locations ALTER COLUMN postal_code SET DEFAULT '';
ALTER TABLE store_locations ALTER COLUMN postal_code DROP NOT NULL;

COMMENT ON TABLE store_locations IS 'Physical store locations for franchise operations';


-- ============================================================================
-- SEED DATA: Application Configuration
-- ============================================================================

-- Feature Flags (managed via ApiLogicServer)
INSERT INTO app_config (config_key, config_value, config_type, category, description, is_sensitive, is_required, default_value) VALUES
('enable_ai_search', 'true', 'boolean', 'features', 'Enable AI-powered prospect search', false, false, 'true'),
('enable_demographics', 'true', 'boolean', 'features', 'Enable demographics visualization', false, false, 'true'),
('enable_osm_data', 'true', 'boolean', 'features', 'Enable OpenStreetMap data integration', false, false, 'true'),
('enable_prospect_scoring', 'true', 'boolean', 'features', 'Enable AI-based prospect scoring', false, false, 'true'),
('enable_market_analysis', 'true', 'boolean', 'features', 'Enable AI market analysis for prospects', false, false, 'true');

-- Display/UI Settings (user preferences stored in database)
INSERT INTO app_config (config_key, config_value, config_type, category, description, is_sensitive, is_required, default_value) VALUES
('default_map_zoom', '12', 'integer', 'display', 'Default zoom level for map views', false, false, '12'),
('default_search_radius', '5', 'integer', 'display', 'Default search radius in miles', false, false, '5'),
('results_per_page', '25', 'integer', 'display', 'Number of results per page in lists', false, false, '25'),
('map_tile_provider', 'openstreetmap', 'string', 'display', 'Map tile provider: openstreetmap, mapbox', false, false, 'openstreetmap'),
('theme', 'light', 'string', 'display', 'UI theme: light, dark', false, false, 'light');

-- Business Rules (configurable thresholds)
INSERT INTO app_config (config_key, config_value, config_type, category, description, is_sensitive, is_required, default_value) VALUES
('min_prospect_score', '50', 'integer', 'business', 'Minimum score to qualify as a prospect', false, false, '50'),
('hot_lead_threshold', '80', 'integer', 'business', 'Score threshold for hot lead classification', false, false, '80'),
('geocoding_cache_minutes', '1440', 'integer', 'business', 'Cache duration for geocoding results (minutes)', false, false, '1440'),
('max_search_results', '100', 'integer', 'business', 'Maximum results returned from search', false, false, '100');

-- System Settings (runtime state)
-- These reference the first Denver franchisee and store for default app state
INSERT INTO app_config (config_key, config_value, config_type, category, description, is_sensitive, is_required, default_value) VALUES
('current_franchisee_id', 'c1000000-0000-0000-0000-000000000001', 'string', 'system', 'Currently selected franchisee ID', false, false, ''),
('current_store_id', 'd1000000-0000-0000-0000-000000000001', 'string', 'system', 'Currently selected store location ID', false, false, '');


-- ============================================================================
-- SEED DATA: Industries
-- ============================================================================

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
('Restaurants', '722511', 3, '$100-$300', ARRAY['Off-peak catering']);


-- ============================================================================
-- SEED DATA: Sample Tags
-- ============================================================================

INSERT INTO tags (name, color, description) VALUES
('hot-lead', '#e74c3c', 'High priority prospect requiring immediate attention'),
('decision-maker-contact', '#27ae60', 'Has direct contact with decision maker'),
('large-events', '#3498db', 'Known for hosting large events'),
('recurring-potential', '#9b59b6', 'Potential for recurring orders'),
('competitor-customer', '#f39c12', 'Currently using competitor services'),
('referral', '#1abc9c', 'Came through referral'),
('needs-follow-up', '#e67e22', 'Requires follow-up action'),
('price-sensitive', '#95a5a6', 'Budget-conscious prospect');


-- ============================================================================
-- SEED DATA: Regions
-- ============================================================================

INSERT INTO regions (id, name, code, country_code, timezone) VALUES
('a1000000-0000-0000-0000-000000000001'::uuid, 'Western Region', 'WEST', 'US', 'America/Los_Angeles'),
('a1000000-0000-0000-0000-000000000002'::uuid, 'Central Region', 'CENTRAL', 'US', 'America/Chicago'),
('a1000000-0000-0000-0000-000000000003'::uuid, 'Eastern Region', 'EAST', 'US', 'America/New_York'),
('a1000000-0000-0000-0000-000000000004'::uuid, 'Southern Region', 'SOUTH', 'US', 'America/Chicago');


-- ============================================================================
-- SEED DATA: Territories (using simple lat/lng instead of PostGIS)
-- ============================================================================

INSERT INTO territories (id, region_id, name, code, description, center_latitude, center_longitude, radius_miles, is_active) VALUES
-- Western Region
('b1000000-0000-0000-0000-000000000001'::uuid, 'a1000000-0000-0000-0000-000000000001'::uuid,
 'San Francisco Metro', 'SF-METRO', 'San Francisco Bay Area territory',
 37.7749, -122.4194, 25.0, true),
('b1000000-0000-0000-0000-000000000002'::uuid, 'a1000000-0000-0000-0000-000000000001'::uuid,
 'Los Angeles Downtown', 'LA-DT', 'Downtown Los Angeles territory',
 34.0522, -118.2437, 15.0, true),
('b1000000-0000-0000-0000-000000000003'::uuid, 'a1000000-0000-0000-0000-000000000001'::uuid,
 'Seattle Metro', 'SEA-METRO', 'Greater Seattle area',
 47.6062, -122.3321, 20.0, true),

-- Central Region
('b1000000-0000-0000-0000-000000000004'::uuid, 'a1000000-0000-0000-0000-000000000002'::uuid,
 'Denver Metro', 'DEN-METRO', 'Denver metropolitan area',
 39.7392, -104.9903, 20.0, true),
('b1000000-0000-0000-0000-000000000005'::uuid, 'a1000000-0000-0000-0000-000000000002'::uuid,
 'Chicago Loop', 'CHI-LOOP', 'Chicago downtown and surrounding areas',
 41.8781, -87.6298, 15.0, true),
('b1000000-0000-0000-0000-000000000006'::uuid, 'a1000000-0000-0000-0000-000000000002'::uuid,
 'Austin Metro', 'AUS-METRO', 'Austin Texas metropolitan area',
 30.2672, -97.7431, 18.0, true),

-- Eastern Region
('b1000000-0000-0000-0000-000000000007'::uuid, 'a1000000-0000-0000-0000-000000000003'::uuid,
 'New York City', 'NYC', 'New York City five boroughs',
 40.7128, -74.0060, 15.0, true),
('b1000000-0000-0000-0000-000000000008'::uuid, 'a1000000-0000-0000-0000-000000000003'::uuid,
 'Boston Metro', 'BOS-METRO', 'Greater Boston area',
 42.3601, -71.0589, 18.0, true),

-- Southern Region
('b1000000-0000-0000-0000-000000000009'::uuid, 'a1000000-0000-0000-0000-000000000004'::uuid,
 'Atlanta Metro', 'ATL-METRO', 'Atlanta metropolitan area',
 33.7490, -84.3880, 22.0, true),
('b1000000-0000-0000-0000-000000000010'::uuid, 'a1000000-0000-0000-0000-000000000004'::uuid,
 'Miami Metro', 'MIA-METRO', 'Miami-Dade area',
 25.7617, -80.1918, 20.0, true);


-- ============================================================================
-- SEED DATA: Franchisees
-- ============================================================================

INSERT INTO franchisees (id, business_name, dba_name, franchise_number, owner_first_name, owner_last_name,
                         email, phone, address_line1, city, state_province, postal_code,
                         latitude, longitude, start_date, is_active) VALUES
('c1000000-0000-0000-0000-000000000001'::uuid,
 'Rocky Mountain Catering LLC', 'Mountain Fresh Catering', 'FRA-001',
 'John', 'Smith', 'john.smith@rmcatering.com', '(303) 555-0101',
 '1600 California St', 'Denver', 'CO', '80202',
 39.7456, -104.9885, '2022-01-15', true),

('c1000000-0000-0000-0000-000000000002'::uuid,
 'Bay Area Gourmet Inc', 'SF Gourmet Catering', 'FRA-002',
 'Sarah', 'Johnson', 'sarah@sfgourmet.com', '(415) 555-0202',
 '555 Market St', 'San Francisco', 'CA', '94105',
 37.7909, -122.3998, '2021-06-01', true),

('c1000000-0000-0000-0000-000000000003'::uuid,
 'Windy City Eats LLC', 'Chicago Corporate Catering', 'FRA-003',
 'Michael', 'Davis', 'mdavis@windycityeats.com', '(312) 555-0303',
 '233 S Wacker Dr', 'Chicago', 'IL', '60606',
 41.8789, -87.6359, '2020-09-01', true);


-- ============================================================================
-- SEED DATA: Franchisee Territory Assignments
-- ============================================================================

INSERT INTO franchisee_territories (franchisee_id, territory_id, is_primary, assigned_date) VALUES
('c1000000-0000-0000-0000-000000000001'::uuid, 'b1000000-0000-0000-0000-000000000004'::uuid, true, '2022-01-15'),
('c1000000-0000-0000-0000-000000000002'::uuid, 'b1000000-0000-0000-0000-000000000001'::uuid, true, '2021-06-01'),
('c1000000-0000-0000-0000-000000000003'::uuid, 'b1000000-0000-0000-0000-000000000005'::uuid, true, '2020-09-01');


-- ============================================================================
-- SEED DATA: Store Locations
-- ============================================================================

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
 41.8789, -87.6359, 'nominatim', CURRENT_TIMESTAMP, 12.0, '(312) 555-0303', true, true);


-- ============================================================================
-- SEED DATA: Sample Prospects
-- ============================================================================

INSERT INTO prospects (id, territory_id, franchisee_id, business_name, business_type,
                       employee_count, employee_count_range, address_line1, city, state_province,
                       postal_code, latitude, longitude, primary_phone, website,
                       status, data_source, is_verified) VALUES
-- Denver Prospects
('e1000000-0000-0000-0000-000000000001'::uuid,
 'b1000000-0000-0000-0000-000000000004'::uuid, 'c1000000-0000-0000-0000-000000000001'::uuid,
 'TechStart Colorado', 'Technology Company', 150, 'medium',
 '1801 California St', 'Denver', 'CO', '80202',
 39.7489, -104.9878, '(303) 555-1001', 'https://techstartco.example.com', 'new', 'openstreetmap', true),

('e1000000-0000-0000-0000-000000000002'::uuid,
 'b1000000-0000-0000-0000-000000000004'::uuid, 'c1000000-0000-0000-0000-000000000001'::uuid,
 'Colorado Healthcare Partners', 'Healthcare Facility', 320, 'large',
 '1635 Aurora Ct', 'Denver', 'CO', '80045',
 39.7456, -104.8372, '(303) 555-1002', 'https://cohealthpartners.example.com', 'contacted', 'openstreetmap', true),

('e1000000-0000-0000-0000-000000000003'::uuid,
 'b1000000-0000-0000-0000-000000000004'::uuid, 'c1000000-0000-0000-0000-000000000001'::uuid,
 'Mile High Law Group', 'Law Firm', 85, 'medium',
 '1700 Broadway', 'Denver', 'CO', '80290',
 39.7436, -104.9872, '(303) 555-1003', 'https://milehighlaw.example.com', 'qualified', 'openstreetmap', true),

-- San Francisco Prospects
('e1000000-0000-0000-0000-000000000004'::uuid,
 'b1000000-0000-0000-0000-000000000001'::uuid, 'c1000000-0000-0000-0000-000000000002'::uuid,
 'Bay Innovations Inc', 'Technology Company', 280, 'large',
 '101 California St', 'San Francisco', 'CA', '94111',
 37.7929, -122.3984, '(415) 555-2001', 'https://bayinnovations.example.com', 'new', 'openstreetmap', true),

('e1000000-0000-0000-0000-000000000005'::uuid,
 'b1000000-0000-0000-0000-000000000001'::uuid, 'c1000000-0000-0000-0000-000000000002'::uuid,
 'Pacific Financial Advisors', 'Financial Services', 120, 'medium',
 '425 Market St', 'San Francisco', 'CA', '94105',
 37.7912, -122.3985, '(415) 555-2002', 'https://pacificfa.example.com', 'proposal_sent', 'openstreetmap', true),

-- Chicago Prospects
('e1000000-0000-0000-0000-000000000006'::uuid,
 'b1000000-0000-0000-0000-000000000005'::uuid, 'c1000000-0000-0000-0000-000000000003'::uuid,
 'Midwest Corporate Holdings', 'Corporate Office', 450, 'large',
 '311 S Wacker Dr', 'Chicago', 'IL', '60606',
 41.8776, -87.6363, '(312) 555-3001', 'https://midwestcorp.example.com', 'new', 'openstreetmap', true),

('e1000000-0000-0000-0000-000000000007'::uuid,
 'b1000000-0000-0000-0000-000000000005'::uuid, 'c1000000-0000-0000-0000-000000000003'::uuid,
 'Chicago Medical Center', 'Healthcare Facility', 890, 'enterprise',
 '710 N Lake Shore Dr', 'Chicago', 'IL', '60611',
 41.8953, -87.6171, '(312) 555-3002', 'https://chicagomedical.example.com', 'qualified', 'openstreetmap', true);


-- ============================================================================
-- SEED DATA: Prospect Scores
-- ============================================================================

INSERT INTO prospect_scores (prospect_id, total_score, score_grade, fit_score, engagement_score,
                            catering_potential_score, proximity_score, score_source, model_version, confidence) VALUES
('e1000000-0000-0000-0000-000000000001'::uuid, 82, 'B', 85, 75, 88, 90, 'ai_model', '1.0.0', 0.85),
('e1000000-0000-0000-0000-000000000002'::uuid, 91, 'A', 95, 88, 92, 85, 'ai_model', '1.0.0', 0.92),
('e1000000-0000-0000-0000-000000000003'::uuid, 78, 'B', 80, 72, 82, 88, 'ai_model', '1.0.0', 0.78),
('e1000000-0000-0000-0000-000000000004'::uuid, 88, 'B', 90, 82, 90, 85, 'ai_model', '1.0.0', 0.88),
('e1000000-0000-0000-0000-000000000005'::uuid, 75, 'B', 78, 70, 80, 82, 'ai_model', '1.0.0', 0.75),
('e1000000-0000-0000-0000-000000000006'::uuid, 94, 'A', 96, 90, 95, 92, 'ai_model', '1.0.0', 0.94),
('e1000000-0000-0000-0000-000000000007'::uuid, 96, 'A', 98, 92, 98, 88, 'ai_model', '1.0.0', 0.96);


-- ============================================================================
-- SEED DATA: Territory Demographics
-- ============================================================================

INSERT INTO territory_demographics (territory_id, total_population, population_density, median_age,
                                   total_households, median_household_income, total_businesses,
                                   total_employees, business_density, data_year, data_source) VALUES
('b1000000-0000-0000-0000-000000000004'::uuid, 716492, 4521.3, 34.5, 305678, 85432.00,
 42350, 528900, 267.8, 2024, 'US Census Bureau'),
('b1000000-0000-0000-0000-000000000001'::uuid, 873965, 18634.2, 38.2, 352890, 126750.00,
 68420, 892340, 1458.6, 2024, 'US Census Bureau'),
('b1000000-0000-0000-0000-000000000005'::uuid, 2693976, 11841.8, 35.8, 1087650, 78650.00,
 125680, 1892450, 552.4, 2024, 'US Census Bureau');


-- ============================================================================
-- HELPER FUNCTIONS FOR APILOGICSERVER
-- ============================================================================

-- ---------------------------------------------------------------------------
-- Function: Get app config value by key
-- ---------------------------------------------------------------------------
CREATE OR REPLACE FUNCTION get_app_config(p_key VARCHAR)
RETURNS TEXT AS $$
DECLARE
    v_value TEXT;
BEGIN
    SELECT config_value INTO v_value
    FROM app_config
    WHERE config_key = p_key;

    IF v_value IS NULL THEN
        SELECT default_value INTO v_value
        FROM app_config
        WHERE config_key = p_key;
    END IF;

    RETURN v_value;
END;
$$ LANGUAGE plpgsql;

COMMENT ON FUNCTION get_app_config IS 'Get application configuration value by key, returns default if null';

-- ---------------------------------------------------------------------------
-- Function: Set app config value
-- ---------------------------------------------------------------------------
CREATE OR REPLACE FUNCTION set_app_config(p_key VARCHAR, p_value TEXT)
RETURNS BOOLEAN AS $$
BEGIN
    UPDATE app_config
    SET config_value = p_value,
        updated_at = CURRENT_TIMESTAMP
    WHERE config_key = p_key;

    RETURN FOUND;
END;
$$ LANGUAGE plpgsql;

COMMENT ON FUNCTION set_app_config IS 'Set application configuration value by key';
