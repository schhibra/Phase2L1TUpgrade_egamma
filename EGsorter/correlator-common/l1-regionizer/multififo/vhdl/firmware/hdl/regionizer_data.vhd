library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

package regionizer_data is 
    type tkobj is record
        pt : unsigned(13 downto 0);
        eta : signed(9 downto 0);
        phi : signed(9 downto 0);
        deta : signed(5 downto 0);
        dphi : unsigned(6 downto 0);
        charge: std_logic;
        rest : std_logic_vector(23 downto 0);
    end record;
    type particle is record
        pt : unsigned(13 downto 0);
        eta : signed(9 downto 0);
        phi : signed(9 downto 0);
        rest : std_logic_vector(37 downto 0);
    end record;
    type glbparticle is record
        pt : unsigned(13 downto 0);
        eta : signed(11 downto 0);
        phi : signed(10 downto 0);
        rest : std_logic_vector(34 downto 0);
    end record;
    type anyparticle is record
        pt : unsigned(13 downto 0);
        rest : std_logic_vector(57 downto 0);
    end record;
    type pfregion is record
        etaCenter : signed(11 downto 0);
        phiCenter : signed(10 downto 0);
        etaHalfWidth : signed(9 downto 0);
        phiHalfWidth : signed(9 downto 0);
        etaExtra : signed(9 downto 0);
        phiExtra : signed(9 downto 0);
    end record;

    subtype word64 is std_logic_vector(63 downto 0);
    subtype word65 is std_logic_vector(64 downto 0);
    subtype word72 is std_logic_vector(71 downto 0);

    function tkobj_to_particle(p : tkobj) return particle;
    function particle_to_tkobj(d : particle) return tkobj;
    function particle_to_w72(p : particle) return word72;
    function w72_to_particle(d : word72) return particle;
    function null_particle return particle;
    function glbparticle_to_w72(p : glbparticle) return word72;
    function w72_to_glbparticle(d : word72) return glbparticle;
    function null_glbparticle return glbparticle;
    function anyparticle_to_w72(p : anyparticle) return word72;
    function w72_to_anyparticle(d : word72) return anyparticle;
    function particle_to_any(p : particle) return anyparticle;
    function glbparticle_to_any(p : glbparticle) return anyparticle;
    function any_to_particle(p: anyparticle) return particle;
    function any_to_glbparticle(p: anyparticle) return glbparticle;
    function null_anyparticle return anyparticle;
    function pfregion_to_w72(p : pfregion) return word72;
    function w72_to_pfregion(d : word72) return pfregion;

    type tkobjs    is array(natural range <>) of tkobj;
    type particles is array(natural range <>) of particle;
    type w64s      is array(natural range <>) of word64;
    type w65s      is array(natural range <>) of word65;
    type w72s      is array(natural range <>) of word72;
    type glbparticles is array(natural range <>) of glbparticle;
    type anyparticles is array(natural range <>) of anyparticle;

    constant ENDCAP_ETA_CENTER : natural := 459;
    constant PHI_SHIFT_INT : natural := 160; -- 2*pi/9, size of a phi nonant, track finder sector or fiducial part of one PF region
    constant PHI_SHIFT : signed(9 downto 0) := to_signed(160, 10); -- 2*pi/9, size of a phi nonant, track finder sector or fiducial part of one PF region
    constant PHI_BORDER : signed(9 downto 0) := to_signed(57, 10); -- 0.25 (0.30 would be 69) 
    constant PHI_MARGIN_POS : signed(9 downto 0) := to_signed(+(160/2-57), 10);  -- half-width of fiducial MINUS border (half-size of gap are between sector N and sector N+2)
    constant PHI_MARGIN_NEG : signed(9 downto 0) := to_signed(-(160/2-57), 10);  -- same but with negative sign
    constant PHI_HALFWIDTH_POS : signed(9 downto 0) := to_signed(+(160/2+57), 10); -- half size of a full region (fiducial PLUS border)
    constant PHI_HALFWIDTH_NEG : signed(9 downto 0) := to_signed(-(160/2+57), 10);  
    constant PHI_HALFWIDTH_FID : signed(9 downto 0) := to_signed(+(160/2), 10);   -- half size of a full region (fiducial only)
    constant ETA_HALFWIDTH_POS : signed(9 downto 0) := to_signed(+(230/2+57), 10); -- half size of a full region (fiducial PLUS border)
    constant ETA_HALFWIDTH_NEG : signed(9 downto 0) := to_signed(-(230/2+57), 10);  
    constant ETA_HALFWIDTH_FID : signed(9 downto 0) := to_signed(+(230/2), 10); -- half size of a full region (fiducial only)

    constant PHI_CALOSHIFT    : signed(9 downto 0) := to_signed( 480,        10);  -- 2*pi/3, size of an HGCal sector
    constant PHI_CALOSHIFT1   : signed(9 downto 0) := to_signed( 320,        10);  -- 2*pi/3 - 2*pi/9, distance between center of hgcal sector 1 and pf region 1 = 2 * size of a phi nonant
    constant PHI_CALOEDGE_POS : signed(9 downto 0) := to_signed(+(480/2-57), 10);  -- +(half-size of calo sector)-border
    constant PHI_CALOEDGE_NEG : signed(9 downto 0) := to_signed(-(480/2-57), 10);  -- -(half-size of calo sector)+border

    constant PHI_PI  : signed(11 downto 0) := to_signed(+PHI_SHIFT_INT*9/2, 12);
    constant PHI_MPI : signed(11 downto 0) := to_signed(-PHI_SHIFT_INT*9/2, 12);
    constant PHI_2PI : signed(11 downto 0) := to_signed( PHI_SHIFT_INT*9,   12);
    constant PHI_M2PI: signed(11 downto 0) := to_signed(-PHI_SHIFT_INT*9,   12);

    constant ETASHIFT_TK   : signed(9 downto 0) := to_signed(-172, 10);
    constant ETASHIFT_CALO : signed(9 downto 0) := to_signed( +57, 10);
    constant ETATK_HALFWIDTH_POS : signed(10 downto 0) := to_signed(+(230/2+57)+172, 11); -- extend by one bit to avoid wrap-around
    constant ETATK_HALFWIDTH_NEG : signed(10 downto 0) := to_signed(-(230/2+57)+172, 11);
    constant ETACALO_HALFWIDTH_POS : signed(10 downto 0) := to_signed(+(230/2+57)-57, 11); -- extend by one bit to avoid wrap-around
    constant ETACALO_HALFWIDTH_NEG : signed(10 downto 0) := to_signed(-(230/2+57)-57, 11);

    constant PFII : natural := 6;
    constant PFII240 : natural := 4;
    constant NPFREGIONS : natural := 9;

    constant NCLK360 : natural := 54;

    constant NTKSECTORS : natural := 9;
    constant NTKFIBERS : natural := 2;
    constant NTKFIFOS : natural := NTKFIBERS*3;
    constant NTKSORTED : natural := 30;
    constant NTKSTREAM : natural := (NTKSORTED+PFII240-1)/PFII240;

    constant NCALOSECTORS : natural := 3;
    constant NCALOFIBERS : natural := 4;
    constant NCALOFIFO0 : natural := NCALOFIBERS;
    constant NCALOFIFO12 : natural := 2*NCALOFIBERS;
    constant NCALOFIFOS : natural := NCALOFIFO0+2*NCALOFIFO12;
    constant NCALOSORTED : natural := 20;
    constant NCALOSTREAM : natural := (NCALOSORTED+PFII240-1)/PFII240;

    constant NMUFIBERS : natural := 2;
    constant NMUSORTED : natural := 4;
    constant NMUSTREAM : natural := (NMUSORTED+PFII240-1)/PFII240;

    constant TKDELAY : natural := 2;
    constant MUDELAY   : natural := 3;
    constant CALODELAY : natural := 1;

    constant NEMCALOSORTED : natural := 10;
    constant NEMCALOSTREAM : natural := (NEMCALOSORTED+PFII240-1)/PFII240;

end package;

package body regionizer_data is
    function tkobj_to_particle(p : tkobj) return particle is
        variable ret : particle;
    begin
        ret.pt  := p.pt;
        ret.eta := p.eta;
        ret.phi := p.phi;
        ret.rest(5 downto 0)  := std_logic_vector(p.deta);
        ret.rest(12 downto 6) := std_logic_vector(p.dphi);
        ret.rest(13) := p.charge;
        ret.rest(37 downto 14) := p.rest;
        return ret;
    end tkobj_to_particle;

    function particle_to_tkobj(d : particle) return tkobj is
        variable ret : tkobj;
    begin
        ret.pt  := d.pt;
        ret.eta := d.eta;
        ret.phi := d.phi;
        ret.deta := signed(d.rest(5 downto 0));
        ret.dphi := unsigned(d.rest(12 downto 6));
        ret.charge := d.rest(13);
        ret.rest := d.rest(37 downto 14);
        return ret;
    end particle_to_tkobj;

    function particle_to_w72(p : particle) return word72 is
        variable ret : word72;
    begin
        ret(13 downto  0) := std_logic_vector(p.pt);
        ret(23 downto 14) := std_logic_vector(p.eta);
        ret(33 downto 24) := std_logic_vector(p.phi);
        ret(71 downto 34) := p.rest;
        return ret;
    end particle_to_w72;

    function w72_to_particle(d : word72) return particle is
        variable ret : particle;
    begin
        ret.pt  := unsigned(d(13 downto  0));
        ret.eta :=   signed(d(23 downto 14));
        ret.phi :=   signed(d(33 downto 24));
        ret.rest := d(71 downto 34);
        return ret;
    end w72_to_particle;

    function null_particle return particle is
        variable ret : particle;
    begin
        ret.pt  := to_unsigned(0,  ret.pt'length);
        ret.eta := to_signed(0, ret.eta'length);
        ret.phi := to_signed(0, ret.phi'length);
        ret.rest := (others => '0');
        return ret;
    end null_particle;

    function glbparticle_to_w72(p : glbparticle) return word72 is
        variable ret : word72;
    begin
        ret(13 downto  0) := std_logic_vector(p.pt);
        ret(25 downto 14) := std_logic_vector(p.eta);
        ret(36 downto 26) := std_logic_vector(p.phi);
        ret(71 downto 37) := p.rest;
        return ret;
    end glbparticle_to_w72;

    function w72_to_glbparticle(d : word72) return glbparticle is
        variable ret : glbparticle;
    begin
        ret.pt  := unsigned(d(13 downto  0));
        ret.eta :=   signed(d(25 downto 14));
        ret.phi :=   signed(d(36 downto 26));
        ret.rest := d(71 downto 37);
        return ret;
    end w72_to_glbparticle;

    function null_glbparticle return glbparticle is
        variable ret : glbparticle;
    begin
        ret.pt  := to_unsigned(0,  ret.pt'length);
        ret.eta := to_signed(0, ret.eta'length);
        ret.phi := to_signed(0, ret.phi'length);
        ret.rest := (others => '0');
        return ret;
    end null_glbparticle;

    function anyparticle_to_w72(p : anyparticle) return word72 is
        variable ret : word72;
    begin
        ret(13 downto  0) := std_logic_vector(p.pt);
        ret(71 downto 14) := p.rest;
        return ret;
    end anyparticle_to_w72;

    function w72_to_anyparticle(d : word72) return anyparticle is
        variable ret : anyparticle;
    begin
        ret.pt  := unsigned(d(13 downto  0));
        ret.rest := d(71 downto 14);
        return ret;
    end w72_to_anyparticle;

    function null_anyparticle return anyparticle is
        variable ret : anyparticle;
    begin
        ret.pt  := to_unsigned(0,  ret.pt'length);
        ret.rest := (others => '0');
        return ret;
    end null_anyparticle;

    function particle_to_any(p : particle) return anyparticle is
    begin
        return w72_to_anyparticle(particle_to_w72(p));
    end particle_to_any;

    function glbparticle_to_any(p : glbparticle) return anyparticle is
    begin
        return w72_to_anyparticle(glbparticle_to_w72(p));
    end glbparticle_to_any;

    function any_to_particle(p: anyparticle) return particle is
    begin
        return w72_to_particle(anyparticle_to_w72(p));
    end any_to_particle;

    function any_to_glbparticle(p: anyparticle) return glbparticle is
    begin
        return w72_to_glbparticle(anyparticle_to_w72(p));
    end any_to_glbparticle;

    function pfregion_to_w72(p : pfregion) return word72 is
        variable ret : word72;
    begin
        ret(11 downto  0) := std_logic_vector(p.etaCenter);
        ret(22 downto 12) := std_logic_vector(p.phiCenter);
        ret(32 downto 23) := std_logic_vector(p.etaHalfWidth);
        ret(42 downto 33) := std_logic_vector(p.phiHalfWidth);
        ret(52 downto 43) := std_logic_vector(p.etaExtra);
        ret(62 downto 53) := std_logic_vector(p.phiExtra);
        ret(71 downto 63) := (others => '0');
        return ret;
    end pfregion_to_w72;

    function w72_to_pfregion(d : word72) return pfregion is
        variable ret : pfregion;
    begin
        ret.etaCenter    := signed(d(11 downto  0));
        ret.phiCenter    := signed(d(22 downto 12));
        ret.etaHalfWidth := signed(d(32 downto 23));
        ret.phiHalfWidth := signed(d(42 downto 33));
        ret.etaExtra     := signed(d(51 downto 43));
        ret.phiExtra     := signed(d(62 downto 53));
        return ret;
    end w72_to_pfregion;

end regionizer_data;


